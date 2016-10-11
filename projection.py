input_file = "poi.tsv"
output_file = "poi_projected.tsv"
soutput_file = "poi_rtree.tsv"
PROJECTION_RANGE = 1000
MAX_LAT = 48.24900
MAX_LON = 11.7240

with open(soutput_file, "w") as n:
   with open(input_file) as i:
       with open(output_file, "w") as o:
           for line in i:
               sp = line.split()
               lat = float(sp[2])
               lon = float(sp[3])
               new_lat = lat*PROJECTION_RANGE/MAX_LAT
               new_lon = lon*PROJECTION_RANGE/MAX_LON
               sp_new = "%s %s %s %s\n" % (sp[0], sp[1], str(new_lat), str(new_lon))
               sp_2 = "%s %s %s %s %s\n " % (sp[0], str(new_lat-10),str(new_lat+10), str(new_lon-10), str(new_lon+10))
               n.write(sp_2)
               o.write(sp_new) 