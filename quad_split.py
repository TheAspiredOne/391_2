import operator

d = dict()
index_values = [[(2,25),(5,23)],[(3,20),(7,17)],[(1,13),(4,15)],[(1,3),(4,0)],[(6,24),(9,21)],[(7,20),(9,15)],[(6,8),(13,3)],[(17,22),(20,9)],[(19,12),(24,9)],[(19,8),(23,6)],[(21,25),(26,21)],[(20,17),(30,15)],[(28,16),(25,12)],[(13,22),(17,19)]]
#d = area(J) - area(El I) -area(E2 I) 

for i in range(len(index_values)):
   for j in range(len(index_values[0:(len(index_values)-1)])):
      rect1 = index_values[i]
      rect2 = index_values[j]
      x = [rect1[0][0],rect1[1][0],rect2[0][0], rect2[1][0]]
      y = [rect1[0][1],rect1[1][1],rect2[0][1], rect2[1][1]]
      jx1 = max(x)
      jy1 = max(y)
      jx2 = min(x)
      jy2 = min(y)
      area_rect1 =(abs(rect1[0][0]-rect1[1][0]))*(abs(rect1[0][1]-rect1[1][1]))
      area_rect2 = (abs(rect2[0][0]-rect2[1][0]))*(abs(rect2[0][1]-rect2[1][1]))
      area_j = (abs(jx1-jx2))*(abs(jy1-jy2))
      preperatory = (rect1[0],rect1[1],rect2[0],rect2[1], jx1, jy1, jx2, jy2)
      calculation = area_j - area_rect1 - area_rect2
      d[preperatory] = calculation

largest = max(d.items(), key=operator.itemgetter(1))[0]
