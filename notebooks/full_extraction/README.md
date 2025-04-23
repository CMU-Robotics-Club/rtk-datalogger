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
3. Install RTKLIB (see below)
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

## Special Installation Steps

### Windows Users
1. Download latest version from RTKLIB (https://rtklib.com)
2. Extract the Zip File to somewhere on your hard drive, and copy file path (Either Ctrl + Shift + C, or double click and select "Copy as Path")
3. Open Settings and Search for "Edit Environment Variables for Your Account" ![image](https://github.com/user-attachments/assets/4c383597-7cd0-479c-8b86-4668df919594)
4. Once opened, select "Path" and press "Edit"![image](https://github.com/user-attachments/assets/aed27548-ff0c-4958-994c-b4c7b088093e)
5. Afterwards, select "New" and paste the file path of the RTKLIB into the section ![image](https://github.com/user-attachments/assets/5f62413b-6954-48cb-9fb8-9761a17d6ada)



### (MacOS users) Installing from source
1. Have cmake & make installed (probably need homewbrew - https://brew.sh/)
2. Download [this release of RTKLIB] (https://github.com/rtklibexplorer/RTKLIB/releases/tag/b34k)
3. Edit the file src/options.c to include `#define _DARWIN_C_SOURCE` on line 31 (above `#define _POSIX_C_SOURCE 199506)
4. Run the following sequence of commands:
```shell
cd app/consapp/convbin/gcc
make
sudo make install # Will ask for your password
cd ../../rnx2rtkp/gcc
make
sudo make install # Might ask for your password
```
