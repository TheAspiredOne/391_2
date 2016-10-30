#include <stdio.h>
#include <sqlite3.h>
#include <stdlib.h>
#include <string.h>
#include "nodes.h"
#include "dist.h"

int depth;      // depth of tree
/*
  Callback function to store depth
  */
int depth_result(void *a_param, int argc, char **argv, char **column) {
  depth = atoi(argv[0]);
  return(0);
}

/*
  Parse blob and fill the ABL with children of non-leaf node
*/
void populateBranchList(char* nodeBlob, Node* ABL) {
  // parse the data blob and insert into a temp table
  char *array[512];
  //active branch list

  // strcpy(ins,"");
  int i=0;
  int ABL_i=0;

  unsigned long id;
  float x1, x2, y1, y2;
  float mind, minm;

  array[i] = strtok(nodeBlob," ");

  while(array[i]!=NULL) {
    if (array[i][0] == '{') {
      array[i]++;

     } else if (array[i][strlen(array[i])-1] == '}'){
      array[i][strlen(array[i])-1]='\0';
    }
    char *ptr;
    switch (i%5) {
      case 0 :

        id=strtol(array[i], &ptr,10);
        break;
      case 1 :
        x1=atof(array[i]);
        break;
      case 2 :
        x2=atof(array[i]);
        break;
      case 3 :
        y1=atof(array[i]);
        break;
      case 4 :
        y2=atof(array[i]);
        ABL[ABL_i].id=id;
        ABL[ABL_i].x1=x1;
        ABL[ABL_i].x2=x2;
        ABL[ABL_i].y1=y1;
        ABL[ABL_i].y2=y2;
        ABL_i++;

        // printf("%lu %f %f %f %f\n", id,x1,x2,y1,y2);
        break;
    }

    array[++i] = strtok(NULL," ");
    // printf("%s\n",array[i-1]);
  }
}

/*
  From sqlite obtain the number of non-leaf children nodes and
  the blob containing node MBRs.
*/

int genBranchList(sqlite3 *db, sqlite3_stmt *stmt, Node node, char* nodeBlob) {
  int nChildren=0;
  int rc;
  // get the MBRs contained in the node
  char *sql = sqlite3_mprintf("SELECT rtreenode(2, data) as data, COUNT(p.parentnode) "\
                              "FROM poi_rtree_node n, poi_rtree_parent p "\
                              "WHERE n.nodeno = p.parentnode AND n.nodeno=%lu;", node.id);
  rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
  if (rc != SQLITE_OK) {
    fprintf(stderr, "Preparation failed sql_children: %s\n", sqlite3_errmsg(db));
    sqlite3_close(db);
  }

  while((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
    sprintf(nodeBlob, "%s", sqlite3_column_text(stmt, 0));
    nChildren = sqlite3_column_int(stmt, 1);
  }
  sqlite3_free(sql);
  sqlite3_finalize(stmt);
  return nChildren;
}

/*
  From sqlite obtain the number of leaf children and
  the blob containing the leaf MBRs.
*/
int genChildrenList(sqlite3 *db, sqlite3_stmt *stmt, Node node, char* nodeBlob) {
  int nChildren=0;
  int rc;
  // get the MBRs contained in the node
  char *sql_leaf = sqlite3_mprintf("SELECT rtreenode(2, data) as data, COUNT(p.nodeno) "\
                              "FROM poi_rtree_node n, poi_rtree_rowid p "\
                              "WHERE n.nodeno = p.nodeno AND n.nodeno=%lu;", node.id);

  rc = sqlite3_prepare_v2(db, sql_leaf, -1, &stmt, 0);

  if (rc != SQLITE_OK) {
    fprintf(stderr, "Preparation failed sql_children: %s\n", sqlite3_errmsg(db));
    sqlite3_close(db);
  }

  while((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
    sprintf(nodeBlob, "%s", sqlite3_column_text(stmt, 0));
    nChildren = sqlite3_column_int(stmt, 1);
  }
  sqlite3_free(sql_leaf);
  sqlite3_finalize(stmt);
  return nChildren;
}

/*
  Partition function for quicksort.
*/
int partition( Node a[], int l, int r, Point p) {
   float pivot;
   int i, j;
   Node t;
   pivot = mindist_c(p,a[l]);
  //  printf("Node : %lu Pivot : %f\n", a[l].id, pivot);
   i = l; j = r+1;

   while( 1)
   {
   	do ++i; while( mindist_c(p,a[i]) <= pivot && i <= r );
   	do --j; while( mindist_c(p,a[j]) > pivot );
   	if( i >= j ) break;
   	t = a[i]; a[i] = a[j]; a[j] = t;
   }
   t = a[l]; a[l] = a[j]; a[j] = t;
   return j;
}

/*
  Quicksort nodes in a list 'a' by mindist.
  Referenced from https://www.tutorialspoint.com/data_structures_algorithms/quick_sort_program_in_c.htm
  October 29, 2016
*/
void quickSort( Node a[], int l, int r, Point p)
{
   int j;

   if( l < r )
   {
   	// divide and conquer
        j = partition( a, l, r, p);
       quickSort( a, l, j-1, p);
       quickSort( a, j+1, r, p);
   }

}

int pruneBranchList(Node node, Point point, Nearest NN, Node* ABL, int nChildren) {
  Node t;     // temp node pointer

  // Iterate through all combinations applying pruning strategy 1
  for (int i=0; i<nChildren; i++) {
    for (int j=0; j<nChildren; j++) {
      if (mindist_c(point, ABL[i]) >= minmax_c(point, ABL[j])) {
        // i is pruned, swap it to the end and "shrink" the array max
        t = ABL[i];
        ABL[i]=ABL[nChildren-1];
        ABL[nChildren-1]=t;
        nChildren--;
      }
    }
  }
  quickSort(ABL,0,nChildren,point );
  return nChildren;
}



void nearestNeighborSearch(sqlite3 *db, sqlite3_stmt *stmt, Node currNode, Point point, Nearest NN, int depth_count) {
  Node newNode;   // newNode
  Node *ABL;    // branchList
  int dist, nChildren;
  //  At leaf level - compute distance to actual objects
  //  If Node.type = LEAF
   if (depth_count==depth+1) {
    printf("At the leaf level!!\n");

     //if the depth counter is now beyond the lowest level of non-leaf nodes
        // For i := 1 to Node.count

  //        dist := objectDIST(Point,Node.branch_i.rect)
  //        If (dist < Nearest .dist)
  //          NN.dist := dist
  //          NN.rect= Node.branch_i.rect
   }
  //  Non-leaf level - order, prune and visit nodes
  //  Else
   else {
  //      Generate Active Branch list
      char nodeBlob[9999];

      if (depth_count==depth) {   // at the last level of non-leaf nodes
        printf("Getting leaves:\n");
        nChildren = genChildrenList(db, stmt, currNode, nodeBlob);
      } else {
        printf("Getting children:\n");
        nChildren = genBranchList(db, stmt,  currNode, nodeBlob);
      }

      ABL = malloc(nChildren * sizeof(Node));
      populateBranchList((char*)nodeBlob,ABL);
      //
      printf("===ABL===\n");
      for (int i=0; i<nChildren; i++) {
        printf("%lu ", ABL[i].id);
      } printf("\n");


      printf("===Sort ABL===\n");
  // sortBranchList(branchList)
      quickSort(ABL,0, nChildren, point);
      for (int i=0; i<nChildren; i++) {
        printf("%lu ", ABL[i].id);
      } printf("\n");

  //
  //      Perform Downward Pruning
  //      (may discard all branches)
  //      last = pruneBranchList(Node, Point, Nearest, branchList)
  //      nChildren is updated to size of pruned ABL
      nChildren = pruneBranchList(currNode, point, NN, ABL, nChildren);
      // ABL = realloc(ABL,nChildren* sizeof(Node));
      printf("===Pruned 1 children to %d\n", nChildren);
      for (int i=0; i<nChildren; i++) {
        printf("%lu ", ABL[i].id);
      } printf("\n");
  //      Iterate through the Active Branch List
  //      For i:= 1 to last
        for(int i=0; i<nChildren; i++) {
  //        newNode := Node.branch_branchlist
            newNode = ABL[i];
  //        Recursively visit child nodes
  //        nearestNeighborSearch(newNode, Point, Nearest)
            nearestNeighborSearch(db, stmt, newNode, point, NN, depth_count+1);

  //        Perform upward Pruning
  // //        last := pruneBranchList(Node, Point, Nearest, branchList)
            nChildren = pruneBranchList(currNode, point, NN, ABL, nChildren);
            printf("===Pruned 2 children to %d\n", nChildren);
            for (int i=0; i<nChildren; i++) {
              printf("%lu ", ABL[i].id);
            } printf("\n");
            // ABL = realloc(ABL,nChildren* sizeof(Node));

        }
    }
}


int main(int argc, char **argv){

  if (argc != 3) {
    printf("USAGE: %s <x> <y>\n", argv[0]);
    return(0);
  }

  sqlite3 *db; //the database
  sqlite3_stmt    *stmt; //the query
  char database_name[]= "as2.db";
  char blob[999];
  int rc;

  rc = sqlite3_open(database_name, &db);
  if( rc ){
    fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
    sqlite3_close(db);
    return(1);
  }

  // get the depth of the whole rtree
  char *sql_depth = sqlite3_mprintf("SELECT rtreedepth(data) "\
                                    "FROM poi_rtree_node "\
                                    "WHERE nodeno=1;");

  rc = sqlite3_exec(db, sql_depth, depth_result,0,0);
  sqlite3_free(sql_depth);
  // set the query point
  Point point= (Point){ .x = atof(argv[1]), .y = atof(argv[2])};

  // start at root node, arbitrary node dimensions
  Node nodeN = (Node){ .id=1 }; // Current Node

  // Initialize Nearest neighbor as something really far away
  Nearest nearest={ .dist=9999999 };  // Nearest Neighbor

  // Initialize depth counter
  int depth_count = 0;

  nearestNeighborSearch(db, stmt, nodeN, point, nearest, depth_count);



  // free(point);
  // free(&nearest);
  sqlite3_finalize(stmt);
  sqlite3_close(db);
}
