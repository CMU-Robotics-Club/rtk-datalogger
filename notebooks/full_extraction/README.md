# RTK Full Extraction Pipeline

## Installation
0. (Optional) Copy this folder to somewhere you'll remember it
1. Ensure python is installed (highly recommend running from within VSCode)
2. Create a virtual environment: 
```bash
python3 -m venv rtk # Creates a virtual environment called "rtk"
source rtk/bin/activate # Enters the virtual environment
pip install -r requirements.txt # Installs the required packages
```
3. Install RTKLIB (https://github.com/tomojitakasu/RTKLIB)
- MacOS users will have to modify some Makefiles and build from source
- Linux users will have to build from source
- Windows users can install a prebuilt version (google RTKLIB)


## How to use
1. Download the Hammerschlag Hall Basestaion Data
2. Open this folder with VSCode
3. Open `rtk_extraction.ipynb` and edit the fields in the first code section:
    - BUGGIES: List of rovers that you want to do data analysis on
    - DATE: Date you want to do data analysis for (format: YYYY-MM-DD)
    - CLEANUP: If true, deletes all the temp files created during analysis
    - DEBUG: Adds extra print statements if getting unexpected outputs
4. Run the first 3 code sections. They will create a bunch of directories to
   ensure data can be batch processed easily.
5. Move rover ubx files (firmware uses extension .BIN) to the `raw_ubx` folder
   corresponding to the date in the corresponding rover directories.
6. If zipped, extract the basestation data and move it into the basestation_data
   directory corresponding to the date.
7. Attempt to run all cells in the notebook. If unsuccessful, ask for help.

## Known issues
- API for map of buggy course is broken so displaying shows on a white background

## Windows Users
1. Download latest version from RTKLIB (https://rtklib.com)
2. Extract the Zip File to somewhere on your hard drive, and copy file path (Either Ctrl + Shift + C, or double click and select "Copy as Path")
3. Open Settings and Search for "Edit Environment Variables for Your Account" ![image](https://github.com/user-attachments/assets/4c383597-7cd0-479c-8b86-4668df919594)
4. Once opened, select "Path" and press "Edit"![image](https://github.com/user-attachments/assets/aed27548-ff0c-4958-994c-b4c7b088093e)
5. Afterwards, select "New" and paste the file path of the RTKLIB into the section ![image](https://github.com/user-attachments/assets/5f62413b-6954-48cb-9fb8-9761a17d6ada)



### (MacOS users) Installing from source
1. Edit the below files as per the diff
2. Run `make && make install` from `(RTKLIB)/app/consapp`

```diff
diff --git a/app/consapp/convbin/gcc/makefile b/app/consapp/convbin/gcc/makefile
index 1df0e93..6532465 100644
--- a/app/consapp/convbin/gcc/makefile
+++ b/app/consapp/convbin/gcc/makefile
@@ -11,7 +11,7 @@ OPTIONS= -DTRACE -DENAGLO -DENAQZS -DENAGAL -DENACMP -DENAIRN -DNFREQ=3 -DNEXOBS
 
 CFLAGS = -O3 -ansi -pedantic -Wall -Wno-unused-but-set-variable $(INCLUDE) $(OPTIONS) -g
 
-LDLIBS = -lm -lrt
+LDLIBS = -lm
 
 all  : convbin
 
diff --git a/app/consapp/pos2kml/gcc/makefile b/app/consapp/pos2kml/gcc/makefile
index 032f309..3874383 100644
--- a/app/consapp/pos2kml/gcc/makefile
+++ b/app/consapp/pos2kml/gcc/makefile
@@ -3,7 +3,7 @@
 BINDIR = /usr/local/bin
 SRC    = ../../../../src
 CFLAGS = -Wall -O3 -ansi -pedantic -I$(SRC) -DTRACE
-LDLIBS  = -lm -lrt
+LDLIBS  = -lm
 
 pos2kml    : pos2kml.o convkml.o convgpx.o solution.o geoid.o rtkcmn.o preceph.o
 
diff --git a/app/consapp/rnx2rtkp/gcc/makefile b/app/consapp/rnx2rtkp/gcc/makefile
index 0e33e2d..e76cf05 100644
--- a/app/consapp/rnx2rtkp/gcc/makefile
+++ b/app/consapp/rnx2rtkp/gcc/makefile
@@ -8,7 +8,7 @@ OPTS    = -DTRACE -DENAGLO -DENAQZS -DENAGAL -DENACMP -DENAIRN -DNFREQ=3 -DNEXOB
 
 # for no lapack
 CFLAGS  = -Wall -O3 -ansi -pedantic -Wno-unused-but-set-variable -I$(SRC) $(OPTS)
-LDLIBS  = -lgfortran -lm -lrt
+LDLIBS  = -lm 
 
 #CFLAGS  = -Wall -O3 -ansi -pedantic -Wno-unused-but-set-variable -I$(SRC) -DLAPACK $(OPTS)
 #LDLIBS  = -lm -lrt -llapack -lblas
diff --git a/app/consapp/rtkrcv/gcc/makefile b/app/consapp/rtkrcv/gcc/makefile
index 89ae61c..b6d7830 100644
--- a/app/consapp/rtkrcv/gcc/makefile
+++ b/app/consapp/rtkrcv/gcc/makefile
@@ -7,7 +7,7 @@ CTARGET= -DTRACE -DENAGLO -DENAQZS -DENACMP -DENAGAL -DENAIRN -DNFREQ=3 -DNEXOBS
 #CTARGET= -DENAGLO -DENAQZS -DENACMP -DENAGAL -DENAIRN -DNFREQ=3 -DIERS_MODEL -DSVR_REUSEADDR
 
 CFLAGS = -Wall -O3 -ansi -pedantic -Wno-unused-but-set-variable -I$(SRC) -I.. -DTRACE $(CTARGET) -g
-LDLIBS  = -lm -lrt -lpthread
+LDLIBS  = -lm -lpthread
 #LDLIBS  = ../../../lib/iers/gcc/iers.a -lm -lrt -lpthread
 
 all        : rtkrcv
diff --git a/app/consapp/str2str/gcc/makefile b/app/consapp/str2str/gcc/makefile
index 3d88200..a02c3a6 100644
--- a/app/consapp/str2str/gcc/makefile
+++ b/app/consapp/str2str/gcc/makefile
@@ -9,7 +9,7 @@ CTARGET=
 
 OPTION = -DENAGLO -DENAGAL -DENAQZS -DENACMP -DENAIRN -DTRACE -DNFREQ=3 -DNEXOBS=3 -DSVR_REUSEADDR
 CFLAGS = -Wall -O3 -ansi -pedantic -Wno-unused-but-set-variable -I$(SRC) $(OPTION) $(CTARGET) -g
-LDLIBS  = -lm -lrt -lpthread
+LDLIBS  = -lm -lpthread
 
 all        : str2str
 str2str    : str2str.o stream.o rtkcmn.o solution.o sbas.o geoid.o
```
