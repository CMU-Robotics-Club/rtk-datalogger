convbin scuwuter.ubx
convbin base.ubx

# possibility
rnx2rtkp -o output.pos -p 2 -m 5 -y 2 -sys G,R,E,C -t -c -s , -l 40.442403483 -79.946996633 286.257 scuwuter.obs base.obs scuwuter.nav

# this gets kinda bad results
rnx2rtkp -o output2.pos -i -d 9 -v 2 -p 2 -y 2 -sys G,R,E,C -t -c -s , -l 40.442403483 -79.946996633 286.257 scuwuter.obs base.obs rover.nav

# this also gets decent results
rnx2rtkp -o output3.pos -d 9 -v 2 -p 2 -y 2 -sys G,R,E,C -t -c -s , -l 40.442403483 -79.946996633 286.257 scuwuter.obs base.obs rover.nav
