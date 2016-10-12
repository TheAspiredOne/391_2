//Avery Tan
//assignment1 CMPUT391
#include <stdio.h>
#include <sqlite3.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv){


  sqlite3 *db; //the database
  sqlite3_stmt *stmt; //the update statement
  char database_name[25] = "munich.db";
  //container for assembling our query
  char sql_statement[999];


  /*
  Q9 (15 pts)

  Write a C program, in a file called q9.c that takes an airport IATA 
  code as a parameter and produces the equivalent answer of question 8 
  above for that airport.
  */
  char sql_statement1[] = "SELECT pt.id "\
                            "FROM poi_tag pt, poi_index pi "\
                                  "WHERE pt.id=pi.id "\
                                  "AND pt.key = \"class\" "\
                                  "AND pt.value LIKE \"c%%\" "\
                                  "AND minLat>= "; 

  char sql_statement2[] =" AND maxLat<= ";

  char sql_statement3[] =" AND minLon>= ";

  char sql_statement4[] =" AND maxLon<= ";

  
  //assembling our query                                  
  strcpy(sql_statement, sql_statement1);
  strncat(sql_statement, argv[1], sizeof(argv[1]));
  strncat(sql_statement, sql_statement2, sizeof(sql_statement2));
  strncat(sql_statement, argv[2], sizeof(argv[2]));
  strncat(sql_statement, sql_statement3, sizeof(sql_statement3));
  strncat(sql_statement, argv[3], sizeof(argv[3]));
  strncat(sql_statement, sql_statement4, sizeof(sql_statement4));
  strncat(sql_statement, argv[4], sizeof(argv[4]));


  int rc;


  rc = sqlite3_open(database_name, &db);
  if( rc ){
    fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
    sqlite3_close(db);
    return(1);
  }

  rc = sqlite3_prepare_v2(db, sql_statement, -1, &stmt, 0);

  if (rc != SQLITE_OK) {  
    fprintf(stderr, "Preparation failed: %s\n", sqlite3_errmsg(db));
    sqlite3_close(db);
    return 1;
  }    
  //printing our list
  while((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
    int col;
    for(col=0; col<sqlite3_column_count(stmt)-1; col++) {
      printf("%s|", sqlite3_column_text(stmt, col));
    }
    printf("%s", sqlite3_column_text(stmt, col));
    printf("\n");
  }

  sqlite3_finalize(stmt); //always finalize a statement
}