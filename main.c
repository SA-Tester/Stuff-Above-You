#include <stdio.h>  //printf, scanf, getc, fopen, fclose
#include <string.h> //strncat, strncpy, strlen, strtok
#include <stdlib.h> //atoi, atof, malloc, abs
#include <ctype.h>  //isspace, upper
#include <math.h>   //sin,cos,tan,atan,atan2, rintf
#include <windows.h> //Sleep

// Plot Orbits -----------------------------------------------------------------------------------------------------------
#define WIDTH 60            //X limit
#define HEIGHT 20           //Y limit
const int X = WIDTH/2;      //Origin X
const int Y = HEIGHT/2;     //Origin Y
const int XMAX = WIDTH-X-1;
const int XMIN = -WIDTH-X;
const int YMAX = WIDTH-Y-1;
const int YMIN = -WIDTH-Y;

char grid[HEIGHT][WIDTH];
double satelliteOrbit[60][2]; //longitudes and latitudes of orbit
//------------------------------------------------------------------------------------------------------------------------

//Conversion Constants ---------------------------------------------------------------------------------------------------
const double DE2RA =  0.174532925E-1;   //Degree to Radians
const double TWOPI = 6.2831853;         //2*PI
const double PI = 3.14159265;           //PI
const double E6A = 1.0E-6;              //1.0 X 10^-6
const double XKMPER =  6378.135;        //Radius of Earth
//------------------------------------------------------------------------------------------------------------------------

//Variables used to assign the data after converting from char to double -------------------------------------------------
double i0;      //Inclination
double e0;      //Eccentricity
double omega_0; //Argument of Perigee
double M0;      //Mean anomaly at Epoch
double n0;      //Mean motion (orbits/solar day)
double B;       //drag term (BSTAR)
double ohm0;    //R.A of Ascending Node
char epyr[3];
char epfrday[15];
double velocity;
//------------------------------------------------------------------------------------------------------------------------

//Variables used to gather data of date and time -------------------------------------------------------------------------
int t0_y,t0_m,t0_d,t0_h,t0_mi,t0_s;                                 //Epoch
int iyear, imonth, iday, ihour, imin, isec,savedh,savedm,saveds;    //Input
//------------------------------------------------------------------------------------------------------------------------

//Variables used to decrypt data from TLE --------------------------------------------------------------------------------
char *ob_name;
char *sat_cat_no;
char *classification;
char *launch_year;
char *launch_number;
char *piece_of_launch;
char *epoch_year;
char *epoch_day;
char *fd_mean_motion;
char *sd_mean_motion;
char *drag_term;
char *ephemeris_type;
char *inclination;
char *ohm;
char *eccentricity;
char *arg_perigee;
char *mean_anomaly;
char *mean_motion;
char *rev_no_at_epoch;
char *info[20];
char tle[250];
char satelliteName[5][50] = {"International Space Station (ISS)",
                        "Starlink 61",
                        "Jason-3",
                        "Landsat-8",
                        "National Oceanic and Atmospheric Admin (NOAA)"};
int so; //store selected satellite option
//-----------------------------------------------------------------------------------------------------------------------

//Variables of ECEF to Geodetic Converter -------------------------------------------------------------------------------
double LAT,LON,ALT,savedLAT,savedLON,savedALT;
double offset;
char location[6][15] = {"Badulla", "Colombo", "Kandy","Los Angeles","Tokyo","London"};
double savedLocations[6][2] = {
                                {6.9934,81.0550},
                                {6.9271,79.8612},
                                {7.2906,80.6337},
                                {34.0522,-118.234},
                                {35.6762,139.6503},
                                {51.5072,0.1276}
                            };
double offset_values[6] = {5.30,5.30,5.30,8.0,9.0,0.0};
//-----------------------------------------------------------------------------------------------------------------------

//System Yes or No ------------------------------------------------------------------------------------------------------
char yNn; //yes or no
//-----------------------------------------------------------------------------------------------------------------------

//Function Declarations -------------------------------------------------------------------------------------------------
void fileReader(FILE *fptr);
void getExistingData(void);
void printTLEInfo(void);
void extractInfo(char*);
char *trimSpaces(char*);
void decrypt(void);
void calculate_epoch_times(void);
double gst_time(int, int, int, int, int, int);
double calculate_delta_t(void);
void ecefToGeo(double ,double ,double );
void orbitPropagator(void);
void init_grid(void);
void show_grid(int, int);
int plot(int, int);
void viewBackgroundInfo(void);
void writeToFile(int, double);
void startSystem(void);
//-----------------------------------------------------------------------------------------------------------------------

int main()
{
    char line0[30];     //User Input TLE Line 0
    char line1[100];    //User Input TLE Line 1
    char line2[100];    //User Input TLE Line 2
    char F;             //Feature Selection
    int DO,CN;          //Decrypt Option, City Name

    printf("\n\n");
    startSystem();
    printf("\n\n\t\t\t\t\t_____________________________________\n\n");
    printf("\t\t\t\t\t\t      MAIN MENU\n");
    printf("\t\t\t\t\t_____________________________________\n\n");
    printf("\t\t\t\t\t   I - View Background Information\n");
    printf("\t\t\t\t\t   D - Decrypt TLEs\n");
    printf("\t\t\t\t\t   S - Predict a Satellite\n");
    printf("\t\t\t\t\t   Q - Quit Application\n\n");
    printf("\t\t\t\t\t   Enter a letter to start: ");
    F = toupper(getchar());

    printf("\n");
    system("cls");

    MAIN:switch(F){
    case 'I':
        //Information -------------------------------------------------------------------------------------------------------------------------
        viewBackgroundInfo();
        break;
        //-------------------------------------------------------------------------------------------------------------------------------------

    case 'D':
        //Decrypt Function --------------------------------------------------------------------------------------------------------------------
        printf("\n\n\n");
        printf("\t\t\t\t________________________________\n\n");
        printf("\t\t\t\t     DECRYPT SATELLITE DATA\n");
        printf("\t\t\t\t________________________________\n\n");
        printf("\t\t\t\t1 - View Existing Data\n");
        printf("\t\t\t\t2 - Enter Your Data\n\n");
        printf("\t\t\t\tEnter option value: ");
        scanf("%d",&DO);

        switch(DO){
        case 1:
            system("cls");
            printf("\n\n\n");
            printf("\t\t\t\t_________________________________________________\n\n");
            printf("\n\t\t\t\t        DECRYPT TLES OF FAMOUS SATELLITES\n\n");
            getExistingData();
            system("cls");
            printTLEInfo();

            printf("\n\n\n\t\t\t\tDo you want to continue ? [y/n]: ");
            scanf(" %c",&yNn);
            if(yNn == 'y'){
                system("cls");
                goto MAIN;
                break;
            }
            else{
                exit(0);
                break;
            }

        case 2:
            system("cls");
            printf("\n\n\n");
            printf("\t\tWARNING: Do not include any spaces in Line 0\n\n");
            printf("\t\tEnter TLE Line 0: ");
            scanf(" %[^\n]%*c",line0);          //" %[^\n]%*c" --> To scan along with spaces until a new line
            printf("\t\tEnter TLE Line 1: ");
            scanf(" %[^\n]%*c",line1);
            printf("\t\tEnter TLE Line 2: ");
            scanf(" %[^\n]%*c",line2);

            strcat(tle,line0);  //Concatenate Input lines to get one large TLE
            strcat(tle," ");
            strcat(tle,line1);
            strcat(tle," ");
            strcat(tle,line2);
            strcat(tle," ");
            printf("%s\n",tle);

            extractInfo(tle);
            decrypt();
            system("cls");
            printTLEInfo();

            printf("\n\n\n\t\t\t\tDo you want to continue ? [y/n]: ");
            scanf(" %c",&yNn);
            if(yNn == 'y'){
                system("cls");
                goto MAIN;      //Go to decrypt menu. (Scope of switch is limited)
                break;
            }
            else{
                exit(0);
                break;
            }
        }
        break;
    //-------------------------------------------------------------------------------------------------------------------------------------------

    //Predict Function --------------------------------------------------------------------------------------------------------------------------
    case 'S':
        printf("\t\t\t\t______________________________________________\n\n");
        printf("\t\t\t\t        PREDICT SATELLITE SIGHTINGS\n");
        printf("\t\t\t\t______________________________________________\n\n");

        getExistingData();
        printf("\n\n\t\t\t\t______________________________________________\n\n");
        printf("\t\t\t\t\t    SELECT A LOCATION\n\n");
        printf("\t\t\t\t______________________________________________\n\n");
        printf("\t\t\t\t1 - BADULLA\n");
        printf("\t\t\t\t2 - COLOMBO\n");
        printf("\t\t\t\t3 - KANDY\n");
        printf("\t\t\t\t4 - LOS ANGELES\n");
        printf("\t\t\t\t5 - TOKYO\n");
        printf("\t\t\t\t6 - LONDON\n\n");
        LOC:printf("\t\t\t\tEnter Option Value: ");
        scanf(" %d",&CN);

        //Validate city selection
        if(CN <1 || CN > 6) {
                printf("\n\t\t\t\tYou entered a Wrong Value !\n");
                goto LOC;
        }

        system("cls");
        printf("\n\n\t\t\t\tFor the Accuracy of Information please enter a\n\t\t\t\tdate after 1st January 1957\n\n");
        printf("\t\t\t\tIf the input date is between 3 days from EPCOH,\n\t\t\t\tmore accurate the information will be.\n\n");
        printf("\t\t\t\tTime inputs are taken in 24h format and converted\n\t\t\t\tto UTC time\n\n");

        //Input Date and Validation --------------------------------------------------------------------------------------------------------------------
        YR:printf("\t\t\t\tEnter Year(yyyy)  : ");
        scanf("%d",&iyear);
        if(iyear < 1957 || iyear > 2056){
            //As no satellites existed before 1957 and TLE format should change after 2056
            printf("\n\t\t\t\tWrong Value for Year !\n");
            goto YR;
        }

        M:printf("\t\t\t\tEnter Month(mm)   : ");
        scanf("%d",&imonth);
        if(imonth <= 0 || imonth >= 13){
            printf("\n\t\t\t\tWrong Value for Month !\n");
            goto M;
        }

        D:printf("\t\t\t\tEnter Day(dd)     : ");
        scanf("%d",&iday);
        if(iday <=0){
            printf("\n\t\t\t\tWrong Value for Day !\n");
            goto D;
        }
        else if ((imonth == 1 || imonth == 3 || imonth == 5 || imonth == 7 || imonth == 8 || imonth == 10 || imonth == 12) && (iday > 31)){
            printf("\n\t\t\t\tWrong Value for Day !\n");
            goto D;
        }
        else if ((imonth == 4 || imonth == 6 || imonth == 9 || imonth == 11) && (iday >= 31)){
            printf("\n\t\t\t\tWrong Value for Day !\n");
            goto D;
        }
        else if (imonth == 2 && iyear%4 != 0 && iday >= 29){
            printf("\n\t\t\t\tWrong Value for Day !\n");
            goto D;
        }

        H:printf("\t\t\t\tEnter Hour(HH)    : ");
        scanf("%d",&ihour);
        if(ihour == 0){
            ihour = 24;
            savedh = 0;
        }
        else if(ihour < 0 || ihour >= 24){
            printf("\n\t\t\t\tWrong Value for Hour !\n");
            goto H;
        }
        else{
            savedh = ihour;
        }

        MI:printf("\t\t\t\tEnter Minute(MM)  : ");
        scanf("%d",&imin);
        if(imin < 0 || imin >= 60){
            printf("\n\t\t\t\tWrong Value for Minutes !\n");
            goto MI;
        }

        S:printf("\t\t\t\tEnter Second(ss)  : ");
        scanf("%d",&isec);
        if(isec < 0 || isec >= 60){
            printf("\n\t\t\t\tWrong Value for Seconds !\n");
            goto S;
        }

        offset = offset_values[CN-1];   //Store UTC offset of the chosen city
        savedm = imin;                  //Save input minutes as imin is subjected to alteration
        saveds = isec;
        //----------------------------------------------------------------------------------------------------------------------------------------

        //Converting to UTC (Universal Time Coordinates) Time ------------------------------------------------------------------------------------
        //Normal Time is GMT (Greenwich Meridian Time): UTC is Time at Greenwich Meridian
        double temph,tempmin,ih,im;
        ih = (double)ihour;             //Convert int to double
        im = (double)imin/100;          //Convert minutes to fractional value
        tempmin = modf(offset,&temph);  //Return Fractional part and integer part of offset separately.(let 5.30) temph->int part(5), tempmin->frac. part(30)

        if((im - tempmin) < 0){         //Input min less than offset min
            im += (0.6-tempmin);        //Subtraction of minutes from the hour
            ih -= (temph+1);            //Subtracting the lost hour
        }
        else{                           //Input min greater than offset
            im -= tempmin;              //Direct Subtraction as no need to convert hours to minutes
            ih -= temph;
        }
        ihour = round(ih);              //Return nearest integer as a float. No need to convert as implicit
        imin = round(im*100);
        //-----------------------------------------------------------------------------------------------------------------------------------------

        orbitPropagator();              //To find exact LAT,LON and ALT at input
        savedLAT = LAT*100;             //Correcting the 10^2 anomaly from SGP4
        savedLON = LON*10;              //Correcting the 10^1 anomaly from SGP4
        savedALT = ALT;

        //Plotting the Orbit ----------------------------------------------------------------------------------------------------------------------
        for(int i=0; i<30; i++){        //Calculating Orbit for next 30 mts
            orbitPropagator();
            satelliteOrbit[i][0] = LAT; //Not corrected as this value is convenient to scale for the orbit
            satelliteOrbit[i][1] = LON;

            imin = imin + i;            //Updating minutes forward
        }

        for(int i=-30; i>0; i++){       //Calculating Orbit for last 30 mts
            orbitPropagator();
            satelliteOrbit[i][0] = LAT;
            satelliteOrbit[i][1] = LON;

            imin = imin + i;            //Updating minutes backwards
        }

        double x,y,LLAT,LLON;           //Store Lat of orbit, Lon of orbit, City lat and City Lon
        int TEMPLAT,TEMPLON;            //Plot Scale Variables
        LLAT = savedLocations[CN-1][0];
        LLON = savedLocations[CN-1][1];
        TEMPLAT = rintf(LLAT*0.03 + 5); //Scaling the value to fit the plot. rintf return the nearest integer as an integer
        TEMPLON = rintf(LLON*0.03 + 5);

        init_grid();
        for(int i = 0; i<60; i++){      //Getting stored coordinates
            x = satelliteOrbit[i][0];
            y = satelliteOrbit[i][1];

            plot(rintf((i-30)*0.5+(x)*0.03),rintf((i-30)*0.5+(y)*0.03));    //Call plot function and store values in grid
        }

        printf("\n\n");
        //---------------------------------------------------------------------------------------------------------------------------------------

        //Approximate distance between observer and satellite 2D coords -------------------------------------------------------------------------
        double a = pow((savedLocations[CN-1][0] - savedLAT),2) + pow((savedLocations[CN-1][1] - savedLON),2)*111;
        double b = pow(savedALT,2);
        double approx_dist_bt_ObnSat = sqrt(a+b);
        //---------------------------------------------------------------------------------------------------------------------------------------

        //Display Output ------------------------------------------------------------------------------------------------------------------------
        system("cls");
        printf("\n\n\t\t\t---------------------------------------------------------------------------------------");
        printf("\n\n\t\t\t\t\t\t\t ANALYSIS REPORT\n");
        printf("\n\t\t\t----------------------------------------------------------------------------------------\n");
        printf("\n\t\t\t________________________________________________________________________________________");
        printf("\n\n\t\t\t\t\t   Satellite Name  : %s\n\n\t\t\t\t\t   Location\t   : %s\n",satelliteName[so],location[CN-1]);
        printf("\n\t\t\t________________________________________________________________________________________");
        printf("\n\n\t\t\tLocal Date\t  : %d.%d.%d\t\t\tLocal Time\t   : %d:%d:%d\n",iyear,imonth,iday,savedh,savedm,saveds);
        printf("\n\t\t\t________________________________________________________________________________________\n");
        printf("\n\t\t\tObserver Latitude : %f deg\t\tObserver Longitude : %f deg\n",savedLocations[CN-1][0],savedLocations[CN-1][1]);
        printf("\n\t\t\t________________________________________________________________________________________\n");
        printf("\n\t\t\tSatellite Latitude: %f deg\t\tSatellite Longitude: %f deg\n",savedLAT,savedLON);
        printf("\n\t\t\t________________________________________________________________________________________\n");
        printf("\n\t\t\tSatellite Altitude: %f km \t\tVelocity\t   : %e km/s\n",savedALT,velocity);
        printf("\n\t\t\t________________________________________________________________________________________\n\n");
        printf("\n\t\t\tApprox. Distance Between Observer and Satellite: %f km\n",approx_dist_bt_ObnSat);
        printf("\n\t\t\t________________________________________________________________________________________\n\n\n\n");

        writeToFile(CN,approx_dist_bt_ObnSat);  //Function call to write the output to a text file
        show_grid(TEMPLAT,TEMPLON);             //Display plot
        //---------------------------------------------------------------------------------------------------------------------------------------

        printf("\n\n\n\t\t\t\tDo you want to continue ? [y/n]: ");
        scanf(" %c",&yNn);
        if(yNn == 'y'){
            system("cls");
            goto MAIN;
            break;
        }
        else{
            exit(0);
            break;
        }
    //-------------------------------------------------------------------------------------------------------------------------------------------

    //Quit Program ------------------------------------------------------------------------------------------------------------------------------
    case 'Q':
        exit(0);
        break;
    }
    //-------------------------------------------------------------------------------------------------------------------------------------------
    return 0;
}

//________________________________________ FINAL OUTPUTS AND SYSTEM FUNCTIONS ___________________________________________________________________________

//Staring Animation -----------------------------------------------------------------------------------------------------------------------------
void startSystem(){
    char a = 177, b = 219;

    //Sleep(10000);
    printf("\n\n\n\t\t\t");
    printf("\t\t\tInitializing System...\n\n\t\t\t\t");
    for(int i=0; i<50; i++) printf("%c",a);

    printf("\r\t\t\t\t");
    for(int i=0; i<50; i++){
        printf("%c",b);
        Sleep(200);
    }

    printf("\n\n");
    for(int i=0; i<=100; i++){
        printf("\r\t\t\t\t\t\t    Loading: %d%c",i,'%');
        Sleep(50);
    }
    system("cls");

    Sleep(100);
    printf("\n\n\n");
    printf("\t\t\t\t      _______________                       _______________\n");
    printf("\t\t\t\t    //              //    //`````\\\\       //              //\n");
    printf("\t\t\t\t   //______________//    ||       ||     //______________//\n");
    printf("\t\t\t\t  //              //      \\\\_____//     //              //\n");
    printf("\t\t\t\t //______________//                    //______________//\n\n\n");
    Sleep(250);

    printf("\t\t\t\t\t     ___  _____  _   _  _____  _____\n");
    printf("\t\t\t\t\t    / __||_____|| | | ||_____||_____|\n");
    printf("\t\t\t\t\t    \\__ \\  | |  | |_| ||__|   |__|\n");
    printf("\t\t\t\t\t    |__ /  |_|  \\_____/|_|    |_|\n\n");
    Sleep(250);

    printf("\t\t\t\t\t  _____  _____   _____ __    __ _____\n");
    printf("\t\t\t\t\t |_____|| |___| |  _  |\\ \\  / /|_____|\n");
    printf("\t\t\t\t\t | |_| || |  \\ \\| |_| | \\ \\/ / |___|_\n");
    printf("\t\t\t\t\t |_| |_||_|__|_||_____|  \\_\\/  |_____|\n\n");
    Sleep(250);

    printf("\t\t\t\t\t\t__    __ _____  _   _ \n");
    printf("\t\t\t\t\t\t\\ \\  / /|  _  || | | |\n");
    printf("\t\t\t\t\t\t \\ \\/ / | |_| || | | |\n");
    printf("\t\t\t\t\t\t  |__|  |_____|\\_____/\n\n\n");
    Sleep(250);
}
//-----------------------------------------------------------------------------------------------------------------------------------------------

//Function to display background information ----------------------------------------------------------------------------------------------------
void viewBackgroundInfo(){
    printf("\n\n\t\t\t\t____________________________________________________\n\n");
    printf("\t\t\t\t\t\tBACKGROUND INFORMATION\n");
    printf("\t\t\t\t____________________________________________________\n\n");
    Sleep(1000);                                                                        //Pause Execution for 1 second
    printf("\t\t\t\t>>TLE stands for Two Line Element Sets.\n\n");
    Sleep(2000);
    printf("\t\t\t\t>>Data about any object in orbit can be transmitted \n\t\t\t\t  using this format.\n\n");
    Sleep(2000);
    printf("\t\t\t\t>>This format was 1st introduced by Max Lane in 1960.\n\n");
    Sleep(2000);
    printf("\t\t\t\t>>To determine nature of the object using TLE data \n\t\t\t\t  presence of a General Perturbation Model is vital.\n\n");
    Sleep(2000);
    printf("\n\n\t\t\t\t____________________________________________________\n\n");
    printf("\t\t\t\t\t\t INTERESTING FACTS");
    printf("\n\t\t\t\t____________________________________________________\n\n");
    Sleep(2000);
    printf("\t\t\t\t>>TLEs were first designed for computers which used\n\t\t\t\t  Punch Cards.\n\n");
    Sleep(2000);
    printf("\t\t\t\t>>After 2056 this format must undergo a change for\n\t\t\t\t  date representation");
    printf(" because TLE use 2 digits \n\t\t\t\t  to represent date.\n\n");
    Sleep(4000);
    printf("\t\t\t\t>>Only years from 1957-2056 could be represented \n\t\t\t\t  without a conflict.");
    printf(" 1957 is the year which first \n\t\t\t\t  satellite Sputnik-1 was launched.\n\n");
    Sleep(6000);
    printf("\t\t\t\t>>On an attempt to publish TLE data on CelesTrak \n\t\t\t\t  T.S Kelso found an error in NASA's checksum system.\n");
    printf("\t\t\t\t  Error was due to lack of a + sign in NASA's \n\t\t\t\t  Teletype machines.\n\n");
    Sleep(6000);
    printf("\t\t\t\t>>This error was tracked down to North American \n\t\t\t\t  Aerospace Defense Command (NORAD), which turned out\n");
    printf("\t\t\t\t  that they've updated their computers from BCD\n\t\t\t\t  to EBCIDIC without sending out updates.\n\n");
}
//-----------------------------------------------------------------------------------------------------------------------------------------------

//Function to read files ------------------------------------------------------------------------------------------------------------------------
void fileReader(FILE *fptr){
    char line[100];     //Buffer Size to scan
    if(fptr == NULL){
        printf("\t\t\tSORRY! THIS FILE CANNOT BE OPENED\n\n");
        exit(1);
    }
    else{
        while(fgets(line,sizeof(line),fptr)){       //Scanning line by line
            strcat(tle,line);                       //Concatenating TLE
        }
        for(int i=0; i<strlen(tle);i++){
            if(tle[i] == '\n' || tle[i] == 'P'){    //Converting new line characters and ascii value for new line to spaces
                tle[i] = ' ';
            }
        }
    }
}
//-------------------------------------------------------------------------------------------------------------------------------------------

//Using Stored Data to do further  processing -----------------------------------------------------------------------------------------------
void getExistingData(){
    FILE *fptr;
    char satName;
    printf("\t\t\t\t_________________________________________________\n\n");
    printf("\t\t\t\tI - INTERNATIONAL SPACE STATION (ISS)\n");
    printf("\t\t\t\tS - SPACE X STARLINK 61\n");
    printf("\t\t\t\tJ - JASON-3\n");
    printf("\t\t\t\tL - LANDSAT-8\n");
    printf("\t\t\t\tN - NATIONAL OCEANIC AND ATMOSPHEIRC ADMIN (NOAA)\n\n");
    R:printf("\t\t\t\tEnter Option Value: ");
    scanf(" %c",&satName);         //Space is to ignore the new line from the previous output
    satName = toupper(satName);    //Converting to capital letter

    if(satName == 'I'){fptr = (fopen("data\\ISS.txt","r")); so = 0;}            //Open tle data file. so store the opened file to view full name
    else if(satName == 'S'){fptr = (fopen("data\\STARLINK.txt","r")); so = 1;}
    else if(satName == 'J'){fptr = (fopen("data\\JASON-3.txt","r")); so = 2;}
    else if(satName == 'L'){fptr = (fopen("data\\LANDSAT.txt","r")); so = 3;}
    else if(satName == 'N'){fptr = (fopen("data\\NOAA.txt","r")); so = 4;}
    else{
        printf("\n\t\t\t\tWrong Input !\n");
        goto R;
    }
    fileReader(fptr);
    extractInfo(tle);
    decrypt();
    fclose(fptr);
}
//-------------------------------------------------------------------------------------------------------------------------------------------

//Display Decrypted TLE Information ---------------------------------------------------------------------------------------------------------
void printTLEInfo(){
    printf("\t\t\t\t____________________________________________________\n\n");
    printf("\t\t\t\t\t\t DECRYPTED INFORMATION\n");
    printf("\t\t\t\t____________________________________________________\n\n");
    printf("\t\t\t\t Satellite Name:\t\t\t%s   \n",ob_name);
    printf("\t\t\t\t____________________________________________________\n\n");
    printf("\t\t\t\t Catalog Number:\t\t\t%s   \n",sat_cat_no);
    printf("\t\t\t\t____________________________________________________\n\n");
    printf("\t\t\t\t Classification:\t\t\t%c   \n",*classification);
    printf("\t\t\t\t____________________________________________________\n\n");
    printf("\t\t\t\t Launch Year:\t\t\t\t%s   \n",launch_year);
    printf("\t\t\t\t____________________________________________________\n\n");
    printf("\t\t\t\t Launch Number:\t\t\t\t%s   \n",launch_number);
    printf("\t\t\t\t____________________________________________________\n\n");
    printf("\t\t\t\t Piece of Launch:\t\t\t%s   \n",piece_of_launch);
    printf("\t\t\t\t____________________________________________________\n\n");
    printf("\t\t\t\t Epoch Year:\t\t\t\t%s   \n",epoch_year);
    printf("\t\t\t\t____________________________________________________\n\n");
    printf("\t\t\t\t Epoch Day:\t\t\t\t%s   \n",epoch_day);
    printf("\t\t\t\t____________________________________________________\n\n");
    printf("\t\t\t\t First Derivative of Mean Motion:\t%s   \n",fd_mean_motion);
    printf("\t\t\t\t____________________________________________________\n\n");
    printf("\t\t\t\t Second Derivative of Mean Motion:\t%s   \n",fd_mean_motion);
    printf("\t\t\t\t____________________________________________________\n\n");
    printf("\t\t\t\t Drag Term:\t\t\t\t%s   \n",drag_term);
    printf("\t\t\t\t____________________________________________________\n\n");
    printf("\t\t\t\t Ephemeris Type:\t\t\t%c   \n",*ephemeris_type);
    printf("\t\t\t\t____________________________________________________\n\n");
    printf("\t\t\t\t Inclination:\t\t\t\t%s   \n",inclination);
    printf("\t\t\t\t____________________________________________________\n\n");
    printf("\t\t\t\t Right Ascension of Ascending Node:\t%s   \n",ohm);
    printf("\t\t\t\t____________________________________________________\n\n");
    printf("\t\t\t\t Eccentricity:\t\t\t\t%s   \n",eccentricity);
    printf("\t\t\t\t____________________________________________________\n\n");
    printf("\t\t\t\t Argument of Perigee:\t\t\t%s   \n",arg_perigee);
    printf("\t\t\t\t____________________________________________________\n\n");
    printf("\t\t\t\t Mean Anomaly:\t\t\t\t%s   \n",mean_anomaly);
    printf("\t\t\t\t____________________________________________________\n\n");
    printf("\t\t\t\t Mean Motion:\t\t\t\t%s   \n",mean_motion);
    printf("\t\t\t\t____________________________________________________\n\n");
    printf("\t\t\t\t Revolution Number at Epoch (Rev/Day):\t%s   \n",rev_no_at_epoch);
    printf("\t\t\t\t____________________________________________________\n\n");

    FILE *f2;
    f2 = fopen("Decrypted_Infomation.txt","w");
    fprintf(f2,"%s","\t\t\t\t____________________________________________________\n\n");
    fprintf(f2,"%s","\t\t\t\t\t\t DECRYPTED INFORMATION\n");
    fprintf(f2,"%s","\t\t\t\t____________________________________________________\n\n");
    fprintf(f2,"%s%s\n","\t\t\t\t Satellite Name:\t\t\t   ",ob_name);
    fprintf(f2,"%s","\t\t\t\t____________________________________________________\n\n");
    fprintf(f2,"%s%s\n","\t\t\t\t Catalog Number:\t\t\t   ",sat_cat_no);
    fprintf(f2,"%s","\t\t\t\t____________________________________________________\n\n");
    fprintf(f2,"%s%c\n","\t\t\t\t Classification:\t\t\t   ",*classification);
    fprintf(f2,"%s","\t\t\t\t____________________________________________________\n\n");
    fprintf(f2,"%s%s\n","\t\t\t\t Launch Year:\t\t\t   ",launch_year);
    fprintf(f2,"%s","\t\t\t\t____________________________________________________\n\n");
    fprintf(f2,"%s%s\n","\t\t\t\t Launch Number:\t\t\t   ",launch_number);
    fprintf(f2,"%s","\t\t\t\t____________________________________________________\n\n");
    fprintf(f2,"%s%s\n","\t\t\t\t Piece of Launch:\t\t\t   ",piece_of_launch);
    fprintf(f2,"%s","\t\t\t\t____________________________________________________\n\n");
    fprintf(f2,"%s%s\n","\t\t\t\t Epoch Year:\t\t\t   ",epoch_year);
    fprintf(f2,"%s","\t\t\t\t____________________________________________________\n\n");
    fprintf(f2,"%s%s\n","\t\t\t\t Epoch Day:\t\t\t   ",epoch_day);
    fprintf(f2,"%s","\t\t\t\t____________________________________________________\n\n");
    fprintf(f2,"%s%s\n","\t\t\t\t First Derivative of Mean Motion:\t   ",fd_mean_motion);
    fprintf(f2,"%s","\t\t\t\t____________________________________________________\n\n");
    fprintf(f2,"%s%s\n","\t\t\t\t Second Derivative of Mean Motion:\t   ",fd_mean_motion);
    fprintf(f2,"%s","\t\t\t\t____________________________________________________\n\n");
    fprintf(f2,"%s%s\n","\t\t\t\t Drag Term:\t\t\t   ",drag_term);
    fprintf(f2,"%s","\t\t\t\t____________________________________________________\n\n");
    fprintf(f2,"%s%c\n","\t\t\t\t Ephemeris Type:\t\t\t   ",*ephemeris_type);
    fprintf(f2,"%s","\t\t\t\t____________________________________________________\n\n");
    fprintf(f2,"%s%s\n","\t\t\t\t Inclination:\t\t\t   ",inclination);
    fprintf(f2,"%s","\t\t\t\t____________________________________________________\n\n");
    fprintf(f2,"%s%s\n","\t\t\t\t Right Ascension of Ascending Node:\t   ",ohm);
    fprintf(f2,"%s","\t\t\t\t____________________________________________________\n\n");
    fprintf(f2,"%s%s\n","\t\t\t\t Eccentricity:\t\t\t   ",eccentricity);
    fprintf(f2,"%s","\t\t\t\t____________________________________________________\n\n");
    fprintf(f2,"%s%s\n","\t\t\t\t Argument of Perigee:\t\t   ",arg_perigee);
    fprintf(f2,"%s","\t\t\t\t____________________________________________________\n\n");
    fprintf(f2,"%s%s\n","\t\t\t\t Mean Anomaly:\t\t\t   ",mean_anomaly);
    fprintf(f2,"%s","\t\t\t\t____________________________________________________\n\n");
    fprintf(f2,"%s%s\n","\t\t\t\t Mean Motion:\t\t\t   ",mean_motion);
    fprintf(f2,"%s","\t\t\t\t____________________________________________________\n\n");
    fprintf(f2,"%s%s\n","\t\t\t\t Revolution Number at Epoch (Rev/Day):\t   ",rev_no_at_epoch);
    fprintf(f2,"%s","\t\t\t\t____________________________________________________\n\n");
    fclose(f2);
}
//-------------------------------------------------------------------------------------------------------------------------------------------

//Write to Text File ------------------------------------------------------------------------------------------------------------------------
void writeToFile(int CN,double approx_dist_bt_ObnSat){
    FILE *wrt = fopen("Final_Report.txt","w");;
    if(wrt == NULL){
        printf("AN ERROR OCCURED. PLEASE TRY AGAIN !\n");
        exit(1);
    }
    else{
        fprintf(wrt,"%s","\n\n\t\t\t________________________________________________________________________________________");
        fprintf(wrt,"%s","\n\n\t\t\t\t\t\t\t ANALYSIS REPORT\n");
        fprintf(wrt,"%s","\n\t\t\t________________________________________________________________________________________\n");
        fprintf(wrt,"%s","\n\t\t\t________________________________________________________________________________________");
        fprintf(wrt,"%s%s%s%s%c","\n\n\t\t\t\t\t\t   Satellite Name  : ",satelliteName[so],"\n\n\t\t\t\t\t\t   Location\t   : ",location[CN-1],'\n');
        fprintf(wrt,"%s","\n\t\t\t________________________________________________________________________________________");
        fprintf(wrt,"%s%d%c%d%c%d%s%d%c%d%c%d","\n\n\t\t\tLocal Date\t  : ",iyear,'.',imonth,'.',iday,"\t\t\tLocal Time\t   : ",savedh,':',savedm,':',saveds);
        fprintf(wrt,"%s","\n\t\t\t________________________________________________________________________________________");
        fprintf(wrt,"%s%f%s%f%s","\n\t\t\tObserver Latitude : ",savedLocations[CN-1][0]," deg\t\tObserver Longitude : ",savedLocations[CN-1][1]," deg\n");
        fprintf(wrt,"%s","\n\t\t\t________________________________________________________________________________________");
        fprintf(wrt,"%s%f%s%f%s","\n\t\t\tSatellite Latitude: ",savedLAT," deg\t\tSatellite Longitude: ",savedLON," deg\n");
        fprintf(wrt,"\n\t\t\t%s","________________________________________________________________________________________");
        fprintf(wrt,"%s%f%s%e%s","\n\t\t\tSatellite Altitude: ",savedALT," km \t\tVelocity\t\t   : ",velocity," km/s\n");
        fprintf(wrt,"%s","\n\t\t\t________________________________________________________________________________________");
        fprintf(wrt,"%s%f%s","\n\t\t\tApprox. Distance Between Observer and Satellite: ",approx_dist_bt_ObnSat," km\n");
        fprintf(wrt,"%s","\n\t\t\t________________________________________________________________________________________");
        fclose(wrt);
    }
}
//-------------------------------------------------------------------------------------------------------------------------------------------

//_______________________________________________________________________________________________________________________________________________________



//________________________________________ TLE PROCESSING AND ORBITAL FUNCTIONS _________________________________________________________________________

//Extract TLE data -------------------------------------------------------------------------------------------------------
void extractInfo(char tle[250]){    //Assign Values of TLE to array info splitting by a space
    int i = 0;
    char *val = strtok(tle," ");

    while(val != NULL){
        info[i] = val;
        i++;
        val = strtok(NULL," "); //A token is a substring extracted from the original string.
    }
}
//------------------------------------------------------------------------------------------------------------------------

//Remove Leading and Trailing White spaces --------------------------------------------------------------------------------
char *trimSpaces(char *str){
    if(str != NULL){
        char *end;
        //Trim Leading Space
        while(isspace((unsigned char)*str)){
            ++str;    //Deallocate memory used by front spaces
        }
        if(*str == 0){
            //All Spaces
            return str;
        }
        //Trim Trailing space
        end = str + strlen(str) - 1;
        while(end > str && isspace((unsigned char)*end)){
            end--;   //Deallocate memory used by trailing spaces
        }
        end[1] = '\0';  //Add new null terminator
    }
    return str;
}
//------------------------------------------------------------------------------------------------------------------------

//Extract TLE data -------------------------------------------------------------------------------------------------------
void decrypt(){
    ob_name = malloc(sizeof(info)/sizeof(char));
    sat_cat_no = malloc(sizeof(info)/sizeof(char));
    classification = malloc(sizeof(char));                  //U: Unclassified, C: Classified, S:Secret
    launch_year = malloc(sizeof(info)/sizeof(char));        //last 2 digits of launch year
    launch_number = malloc(sizeof(info)/sizeof(char));      //launch number of the year
    piece_of_launch = malloc(sizeof(info)/sizeof(char));    //piece of the launch
    epoch_year = malloc(sizeof(info)/sizeof(char));         //Last 2 digits of epoch
    epoch_day = malloc(sizeof(info)/sizeof(char));          //day of year and fractional portion of day
    fd_mean_motion = malloc(sizeof(info)/sizeof(char));     //first derivative of mean motion (ballistic coefficient)
    sd_mean_motion = malloc(sizeof(info)/sizeof(char));     //second derivative of mean motion
    drag_term = malloc(sizeof(info)/sizeof(char));          //drag term or radiation pressure coefficient
    ephemeris_type = malloc(sizeof(char));                  //Mathematical model number
    inclination = malloc(sizeof(info)/sizeof(char));        //degrees
    ohm = malloc(sizeof(info)/sizeof(char));                //Right ascension of Ascending node (degrees)
    eccentricity = malloc(sizeof(info)/sizeof(char));       //elliptic orbits take values from 0-1
    arg_perigee = malloc(sizeof(info)/sizeof(char));        //degrees
    mean_anomaly = malloc(sizeof(info)/sizeof(char));       //degrees
    mean_motion = malloc(sizeof(info)/sizeof(char));        //revolutions per day
    rev_no_at_epoch = malloc(sizeof(info)/sizeof(char));

    for(int i=0; i<strlen(info[0]); i++){
        ob_name[i] = info[0][i];
    }
    ob_name[strlen(info[0])] = '\0';

    strncpy(sat_cat_no,info[2],strlen(info[2])-1);
    sat_cat_no[strlen(sat_cat_no)] = '\0';

    classification = (char*)&info[2][strlen(info[2])-1];

    for(int i=0; i<2; i++){
        launch_year[i] = info[3][i];
        info[3][i] = ' ';
    }
    launch_year[2] = '\0';

    info[3] = trimSpaces(info[3]);
    for(int i=0; i<strlen(info[3])-1; i++){
        launch_number[i] = info[3][i];
        info[3][i] = ' ';
    }
    launch_number[strlen(launch_number)] = '\0';

    info[3] = trimSpaces(info[3]);
    for(int i=0; i<strlen(info[3]); i++){
        piece_of_launch[i] = info[3][i];
    }
    piece_of_launch[1] = '\0';

    strncpy(epoch_year,info[4],2);
    epoch_year[2] = '\0';
    info[4][0] = ' ';
    info[4][1] = ' ';
    info[4] = trimSpaces(info[4]);

    strncpy(epoch_day,info[4],strlen(info[4]));
    epoch_day[strlen(info[4])] = '\0';

    for(int i=0; i<strlen(epoch_year); i++){
        epyr[i] = epoch_year[i];
    }
    epyr[3] = '\0';

    for(int i=0; i<strlen(epoch_day); i++){
        epfrday[i] = epoch_day[i];
    }
    epfrday[15] = '\0';

    for(int i=0; i<strlen(info[5]); i++){
        fd_mean_motion[i] = info[5][i];
    }
    fd_mean_motion[strlen(info[5])] = '\0';

    for(int i=0; i<strlen(info[6]); i++){
        sd_mean_motion[i] = info[6][i];
    }
    sd_mean_motion[strlen(info[6])] = '\0';

    for(int i=0; i<strlen(info[7]); i++){
        drag_term[i] = info[7][i];
    }
    drag_term[strlen(info[7])] = '\0';
    for(int i=0; i<strlen(drag_term); i++){
        if(drag_term[i] == '-'){
            drag_term[i] = 'e';
        }
    }
    char last = drag_term[strlen(drag_term)-1];
    drag_term[strlen(drag_term)-1] = '-';
    drag_term[strlen(drag_term)] = last;
    drag_term[8] = '\0';
    B = atof(drag_term);        //String to float

    ephemeris_type = (char*)(&info[8][0]);

    //Start of Line 2
    for(int i=0; i<strlen(info[12]); i++){
        inclination[i] = info[12][i];
    }
    inclination[strlen(info[12])] = '\0';
    i0 = atof(inclination);

    for(int i=0; i<strlen(info[13]); i++){
        ohm[i] = info[13][i];
    }
    ohm[strlen(info[13])] = '\0';
    ohm0 = atof(ohm);

    for(int i=0; i<strlen(info[14]); i++){
        eccentricity[i] = info[14][i];
    }
    eccentricity[strlen(info[14])] = '\0';
    e0 = atof(eccentricity)/1000000;

    for(int i=0; i<strlen(info[15]); i++){
        arg_perigee[i] = info[15][i];
    }
    arg_perigee[strlen(info[15])] = '\0';
    omega_0 = atof(arg_perigee);

    for(int i=0; i<strlen(info[16]); i++){
        mean_anomaly[i] = info[16][i];
    }
    mean_anomaly[strlen(info[16])] = '\0';
    M0 = atof(mean_anomaly);

    info[17][strlen(info[17])-1] = ' ';
    info[17] = trimSpaces(info[17]);
    for(int i=0; i<strlen(info[17])-5; i++){
        mean_motion[i] = info[17][i];
        info[17][i] = ' ';
    }
    mean_motion[11] = '\0';
    n0 = atof(mean_motion);
    info[17] = trimSpaces(info[17]);

    for(int i=0; i<strlen(info[17]); i++){
        rev_no_at_epoch[i] = info[17][i];
    }
    rev_no_at_epoch[strlen(info[17])] = '\0';
}
//------------------------------------------------------------------------------------------------------------------------

//Convert EPOCH date and time to usual format ----------------------------------------------------------------------------
void calculate_epoch_times(){

    //Calculating the EPOCH year
    if(atoi(epyr)>=0 && atoi(epyr)<=56){
        t0_y = 2000 + atoi(epyr);
    }
    else if(atoi(epyr)>=57 && atoi(epyr)<=99){
        t0_y = 1900 + atoi(epyr);
    }

    //Calculate the EPOCH month
    int nd = (int)atof(epfrday);
    int days_in_months[12] = {31,28,31,30,31,30,31,31,30,31,30,31};
    int tot = 0;
    int c = 0;

    if (nd <= 31){
        t0_m = 1;
        t0_d = nd;
    }
    else{
        while (tot <= nd){
            tot += days_in_months[c];
            c += 1;
        }
        tot -= days_in_months[c-1];
        t0_m = c;
        t0_d = nd - tot;
    }

    //Calculate EPOCH Hours
    double h = (atof(epfrday)-nd)*24;
    t0_h = h;

    //Calculate EPOCH Minutes
    double min = (h - t0_h)*60;
    t0_mi = min;

    //Calculate EPOCH Seconds
    double sec = (min - t0_mi)*60;
    t0_s = sec;
}
//------------------------------------------------------------------------------------------------------------------------

//Get the GST time (Seconds elapsed since 01/01/1970 00:00:00) -----------------------------------------------------------
double gst_time(int years, int months, int days, int hours, int minutes, int seconds){
    calculate_epoch_times();

    int uey = 1970;         //Unix Year

    //Calculating number of days in the elapsed months
    int ndays = 0;
    for(int i=1; i<months; i++){
        if(i % 2 != 0 && i <= 7){
            ndays += 31;
        }
        else if(i % 2 == 0 && i < 7){
            ndays += 30;
        }
        else if(i % 2 != 0 && i > 7){
            ndays += 30;
        }
        else {
            ndays += 31;
        }
    }

    //February in non-leap year and input months is a month after February
    if(months >= 2 && years % 4 != 0){
        ndays -= 2;
    }
    else if(months >= 2 && years % 4 == 0){
        ndays -= 1;
    }

    double unix = (years - uey)*365*24*3600 + (ndays+days)*24*3600 + hours*3600 + minutes * 60 + seconds;
    double jdate = (unix/86400.0) + 2440587.5;

    double tut1 = (jdate - 2451545.0)/36525.0;
    double temp = -6.2E-6 *tut1*tut1*tut1 + 0.093104*tut1*tut1 + (876600.0 * 3600 + 8640184.812866) * tut1 + 67310.54841;
    temp = ((temp*DE2RA) / fmod(240.0,TWOPI))*E6A - 3.7;

    if(temp < 0.0){
        temp += TWOPI;
    }

    return temp;
}
//------------------------------------------------------------------------------------------------------------------------

//Get the Difference between EPOCH and INPUT times -----------------------------------------------------------------------
double calculate_delta_t(){
    int t0,t;
    double delta_t = 0;
    t0 = gst_time(t0_y,t0_m,t0_d,t0_h,t0_mi,t0_s);
    t = gst_time(iyear,imonth,iday,ihour,imin,isec);
    delta_t = ((double)t - (double)t0);
    return delta_t;
}
//------------------------------------------------------------------------------------------------------------------------

//ECEF to GEODETIC -----------------------------------------------------------------------------------------------------------
void ecefToGeo(double eciX,double eciY,double eciZ){
    const double b = 6356.7523142;
    double gmst = gst_time(iyear,imonth,iday,ihour,imin,isec);
    double R = sqrt(eciX*eciX + eciY*eciY + eciZ*eciZ); //equatorial position of satellite projection
    double f = (XKMPER - b)/XKMPER;
    double e2 = 2*f - f*f;
    LON = atan2(eciY,eciX) - gmst;

    while(LON < (-1)*PI){
        LON += TWOPI;
    }

    while (LON > PI){
        LON -= TWOPI;
    }

    int kmax = 20;  //ECEF->GEO max error = 20km
    int k = 0;
    LAT = atan2(eciZ, sqrt(eciX*eciX + eciY*eciY));
    double C;

    while(k < kmax){
        C = 1/(sqrt(1 - e2*sin(LAT)*sin(LAT)));
        LAT = atan2(eciZ + XKMPER*C*e2*sin(LAT),R);
        k += 1;
    }

    ALT = (R / cos(LAT)) - XKMPER*C;
}
//----------------------------------------------------------------------------------------------------------------------------

//Get Satellite Location at the given time -----------------------------------------------------------------------------------
double FMOD2P(double X){
    //Convert an Angle to a value between 0 and 2pi
    double FMOD2P = X;
    double I = FMOD2P/TWOPI;
    FMOD2P = (FMOD2P-I)*TWOPI;
    if(FMOD2P < 0){
        FMOD2P = FMOD2P + TWOPI;
    }
    return FMOD2P;
}

double ACTAN(double SINX, double COSX){
    //tan inverse in radians and between 0 and TWOPI
    double actan = atan2(SINX,COSX);

    if(actan<0){
        actan += TWOPI;
    }
    return actan;

}

void orbitPropagator(){
    //Constants used by orbitProgagator() ---------------------------------------------------------------------------------
    const double XMNPDA = 1440.0;           //Minutes per day
    const double TOTHIRD = 0.66666667;      //2/3
    const double XKE = 0.743669161E-1;      //SGP constant. In relation with universal gravitational constant and mass
    const double CK2 = 5.413080E-4;         //SGP constant. Related with 2nd zonal harmonic of earth
    const double CK4 = 0.62098875E-6;       //SGP constant. Related with 4th zonal harmonic of earth
    const double AE = 1.0;                  //Earth Radii Unit
    const double QOMS2T = 1.88027916E-9;    //SGP4/8 density function
    const double XJ3 = -0.253881E-5;        //Third Zonal Harmonic of Earth
    double S =  1.01222928;                 //SGP4/8 density function

    //from TLE
    double XNO = (n0/XMNPDA)*TWOPI;   //Mean motion
    double XINCL = i0*DE2RA;          //inclination
    double EO = e0;                   //eccentricity
    double BSTAR = B/10;              //BSTAR
    double OMEGAO = omega_0*DE2RA;    // Arg Perigee
    double XMO = M0*DE2RA;            //Mean Anomaly
    double XNODEO = ohm0*DE2RA;       //RA ohm;
    double S4,QOMS24,PERIGEE,PINVSQ;  //SGP4/SGP8 Constants
    double TSINCE = calculate_delta_t();

    //Recover Original Mean Motion and Semi Major Axis ------------------------------------------------------
    double A1 = pow((XKE/XNO),TOTHIRD);
    double COSIO = cos(XINCL);
    double THETA2 = COSIO*COSIO;
    double X3THM1 = 3*THETA2 -1;
    double EOSQ = EO*EO;
    double BETAO2 = 1 - EOSQ;
    double BETAO = sqrt(BETAO2);
    double DEL1=1.5*CK2*X3THM1/(A1*A1*BETAO*BETAO2);
    double AO=A1*(1.-DEL1*(.5*TOTHIRD+DEL1*(1.+134./81.*DEL1)));
    double DELO=1.5*CK2*X3THM1/(AO*AO*BETAO*BETAO2);

    double XNODP=XNO/(1.0+DELO);
    double AODP=AO/(1.0-DELO);
    //-------------------------------------------------------------------------------------------------------

    int ISIMP = 0;  //flag
    if((AODP*(1-EO)/AE) < (220/XKMPER + AE)){
        ISIMP = 1;
    }

    //Calculating SGP4 constant S according to Perigee height -----------------------------------------------
    S4 = S;
    QOMS24 = QOMS2T;
    PERIGEE = (AODP*(1-EO)-AE)*XKMPER;
    if(PERIGEE > 156) goto TEN;
    S4 = PERIGEE - 78;
    if(PERIGEE > 98) goto NINE;
    S4 = 20;
    NINE:QOMS24= pow(((120.-S4)*AE/XKMPER),4);
    S4 = S4/XKMPER + AE;
    TEN:PINVSQ=1./(AODP*AODP*BETAO2*BETAO2);

    double TSI = 1/(AODP-S4);
    double ETA = AODP*EO*TSI;
    double ETASQ = pow(ETA,2);
    double EETA = EO*ETA;
    double PSISQ = (1.0-ETASQ);
    if(PSISQ < 0){
        PSISQ = PSISQ*(-1);
    }
    double COEF = QOMS24*pow(TSI,4);
    double COEF1 = COEF/(pow(PSISQ,3.5));
    double C2 = COEF1*XNODP*(AODP*(1+1.5+ETASQ + EETA*(4+ETASQ)) + 75*(CK2*TSI/PSISQ*X3THM1*(8+3*ETASQ)));

    double C1 = BSTAR*C2;
    double SINIO = sin(XINCL);
    double A30VK2 = -XJ3/CK2*pow(AE,3);
    double C3 = COEF*TSI*A30VK2*XNODP*AE*SINIO/EO;
    double X1MTH2 = 1 - THETA2;
    double C4 = 2*XNODP*COEF*AODP*BETAO2*(ETA*(2+0.5*ETASQ)+EO*(0.5+2*ETASQ)-2*CK2*TSI/(AODP*PSISQ)*(-3*X3THM1*(1-2*EETA+ETASQ*(1.5-0.5*EETA))+0.75*X1MTH2*(2*ETASQ-EETA*(1+ETASQ))*cos(2*OMEGAO)));
    double C5 = 2*COEF1*AODP*BETAO2*(1+2.75*(ETASQ+EETA)+EETA*ETASQ);
    double THETA4 = THETA2*THETA2;
    double TEMP1 = 3.*CK2*PINVSQ*XNODP;
    double TEMP2 = TEMP1*CK2*PINVSQ;
    double TEMP3 = 1.25*CK4*PINVSQ*PINVSQ*XNODP;
    double XMDOT = XNODP + 5*TEMP1*BETAO2*X3THM1+0.625*TEMP2*BETAO2*(13-78*THETA2+137*THETA4);
    double X1M5TH = 1-5*THETA2;
    double OMGDOT = 0.5*TEMP1*X1M5TH+0.625*TEMP2*(7-114*THETA2+395*THETA4)+TEMP3*(3-36*THETA2+49*THETA4);
    double XHDOT1 = -TEMP1*COSIO;
    double XNODOT=XHDOT1+(0.5*TEMP2*(4-19*THETA2)+2.*TEMP3*(3-7*THETA2))*COSIO;
    double OMGCOF = BSTAR*C3*cos(OMEGAO);
    double XMCOF = -TOTHIRD*COEF*BSTAR*AE/EETA;
    double XNODCF = 3.5*BETAO2*XHDOT1*C1;
    double T2COF = 1.5*C1;
    double XLCOF = 0.125*A30VK2*SINIO*(3+5*COSIO)/(1+COSIO);
    double AYCOF = 0.25*A30VK2*SINIO;
    double DELMO = pow(1+ETA*cos(XMO),3);
    double SINMO = sin(XMO);
    double X7THM1 = 7*THETA2-1;

    if(ISIMP == 1) goto HUNDRED;

    double C1SQ = C1*C1;
    double D2 = 4*AODP*TSI*C1SQ;
    double TEMP = D2*TSI*C1/3;
    double D3 = (17.*AODP+S4)*TEMP;
    double D4 = 0.5*TEMP*AODP*TSI*(221*AODP+31*S4)*C1;
    double T3COF = D2+2*C1SQ;
    double T4COF = 0.25*(3.*D3+C1*(12*D2+10*C1SQ));
    double T5COF = 0.2*(3.*D4+12.*C1*D3+6.*D2*D2+15.*C1SQ*(2.*D2+C1SQ));
    //----------------------------------------------------------------------------------------------------------

    //UPDATE FOR SECULAR GRAVITY AND DRAG ----------------------------------------------------------------------
    double XMDF;
    HUNDRED: XMDF = XMO+XMDOT*TSINCE;
    double OMEGADF = OMEGAO+OMGDOT*TSINCE;
    double XNODDF = XNODEO+XNODOT*TSINCE;
    double OMEGA = OMEGADF;
    double XMP = XMDF;
    double TSQ = TSINCE*TSINCE;
    double XNODE = XNODDF+XNODCF*TSQ;
    double TEMPA = 1-C1*TSINCE;
    double TEMPE = BSTAR*C4*TSINCE;
    double TEMPL = T2COF*TSQ;

    if(ISIMP == 1)goto ONEHUNDREDANDTEN;

    double DELOMG = OMGCOF*TSINCE;
    double DELM = XMCOF*(pow(1+ETA*cos(XMDF),3)-DELMO);
    TEMP = DELOMG+DELM;
    XMP = XMDF+TEMP;
    OMEGA = OMEGADF-TEMP;
    double TCUBE = TSQ*TSINCE;
    double TFOUR = TSINCE*TCUBE;
    TEMPA = TEMPA-D2*TSQ-D3*TCUBE-D4*TFOUR;
    TEMPE = TEMPE+BSTAR*C5*(sin(XMP)-SINMO);
    TEMPL = TEMPL + T3COF*TCUBE + TFOUR*(T4COF+TSINCE*T5COF);

    double A;
    ONEHUNDREDANDTEN: A = AODP*pow(TEMPA,2);
    double E = EO-TEMPE;
    double XL = XMP + OMEGA+XNODE+XNODP*TEMPL;
    double BETA = sqrt(1-(E*E));
    double XN = XKE/pow(A,1.5);
    //-----------------------------------------------------------------------------------------------------------

    //LONG PERIOD PERIODICS -------------------------------------------------------------------------------------
    double AXN = E*cos(OMEGA);
    TEMP = 1/(A*BETA*BETA);
    double XLL = TEMP*XLCOF*AXN;
    double AYNL = TEMP*AYCOF;
    double XLT = XL+XLL;
    double AYN = E*sin(OMEGA)+AYNL;
    //-----------------------------------------------------------------------------------------------------------

    //SOLVE KEPLER EQUATION -------------------------------------------------------------------------------------
    double CAPU = FMOD2P(XLT-XNODE);
    TEMP2 = CAPU;
    double SINEPW, COSEPW, TEMP4, TEMP5, TEMP6,TEMP7, EPW;
    int i=0;
    do{
        SINEPW = sin(TEMP2);
        COSEPW = cos(TEMP2);
        TEMP3 = AXN*SINEPW;
        TEMP4 = AYN*COSEPW;
        TEMP5 = AXN*COSEPW;
        TEMP6 = AYN*SINEPW;
        EPW = (CAPU-TEMP4+TEMP3-TEMP2)/(1 - TEMP5-TEMP6)+TEMP2;
        TEMP7 = TEMP2;
        TEMP2 = EPW;
        i++;
    }
    while(i<=10 && (fabs(EPW - TEMP7) > E6A));
    //-----------------------------------------------------------------------------------------------------------

    // SHORT PERIOD PRELIMINARY QUANTITIES ----------------------------------------------------------------------
    double ECOSE = TEMP5+TEMP6;
    double ESINE = TEMP3-TEMP4;
    double ELSQ = AXN*AXN+AYN*AYN;
    TEMP = 1-ELSQ;
    double PL = A*TEMP;
    double R = A*(1-ECOSE);
    TEMP1 = 1/R;
    double RDOT = XKE*sqrt(A)*ESINE*TEMP1;
    double RFDOT = XKE*sqrt(PL)*TEMP1; //fabs
    TEMP2 = A*TEMP1;
    double BETAL = sqrt(TEMP); //Fabs()
    TEMP3 = 1/(1+BETAL);
    double COSU = TEMP2*(COSEPW-AXN+AYN*ESINE*TEMP3);
    double SINU = TEMP2*(SINEPW-AYN-AXN*ESINE*TEMP3);
    double U = ACTAN(SINU,COSU);
    double SIN2U = 2*SINU*COSU;
    double COS2U = 2*COSU*COSU - 1;
    TEMP = 1/(PL);
    TEMP1 = CK2*TEMP;
    TEMP2 = TEMP1*TEMP;
    //-----------------------------------------------------------------------------------------------------------

    //UPDATE FOR SHORT TERM PERIODICS ---------------------------------------------------------------------------
    double RK = R*(1-1.5*TEMP2*BETAL*X3THM1)+.5*TEMP1*X1MTH2*COS2U;
    double UK = U-0.25*TEMP2*X7THM1*SIN2U;
    double XNODEK = XNODE+1.5*TEMP2*COSIO*SIN2U;
    double XINCK = XINCL+1.5*TEMP2*COSIO*SINIO*COS2U;
    double RDOTK = RDOT-XN*TEMP1*X1MTH2*SIN2U;
    double RFDOTK = RFDOT+XN*TEMP1*(X1MTH2*COS2U+1.5*X3THM1);
    //-----------------------------------------------------------------------------------------------------------

    //ORIENTAION VECTORS ----------------------------------------------------------------------------------------
    double SINUK = sin(UK);
    double COSUK = cos(UK);
    double SINIK = sin(XINCK);
    double COSIK = cos(XINCK);
    double SINNOK = sin(XNODEK);
    double COSNOK = cos(XNODEK);
    double XMX = -SINNOK*COSIK;
    double XMY = COSNOK*COSIK;
    double UX = XMX*SINUK+COSNOK*COSUK;
    double UY = XMY*SINUK+SINNOK*COSUK;
    double UZ = SINIK*SINUK;
    double VX = XMX*COSUK-COSNOK*SINUK;
    double VY = XMY*COSUK-SINNOK*SINUK;
    double VZ = SINIK*COSUK;
    //-----------------------------------------------------------------------------------------------------------

    //POSITION AND VELOCITY -------------------------------------------------------------------------------------
    double X = RK*UX;
    double Y = RK*UY;
    double Z = RK*UZ;
    double XDOT = RDOT*UX + RFDOTK*VX;
    double YDOT = RDOTK*UY + RFDOTK*VY;
    double ZDOT = RDOTK*UZ + RFDOTK*VZ;

    X = X*XKMPER/AE;
    Y = Y*XKMPER/AE;
    Z = Z*XKMPER/AE;
    XDOT=XDOT*XKMPER/AE*XMNPDA/86400;
    YDOT=YDOT*XKMPER/AE*XMNPDA/86400;
    ZDOT=ZDOT*XKMPER/AE*XMNPDA/86400;
    //-----------------------------------------------------------------------------------------------------------

    ecefToGeo(X,Y,Z);
    velocity = sqrt(XDOT*XDOT + YDOT*YDOT + ZDOT*ZDOT);
}

//Plot the Orbit ------------------------------------------------------------------------------------------------------------------------------
void init_grid(){
    int x,y;

    //Keep spaces for non-axis coordinates
    for(y=0; y<HEIGHT; y++){
        for(x=0; x<WIDTH; x++){
            grid[y][x] = ' ';
        }
    }

    //Draw y axis for all the points where x coord is X(30) and y coord is between 0-HEIGHT
    for(y=0; y<HEIGHT; y++){
        grid[y][X] = '|';
    }

    //Draw y axis for all the points where y coord is Y(10) and x coord is between 0-WIDTH
    for(x=0; x<WIDTH; x++){
        grid[Y][x] = '-';
    }

    //Origin (0,0)
    grid[Y][X] = '+';
}

void show_grid(int LT,int LO){

    //Initializing File pointer to print the plot
    FILE *graph;
    graph = fopen("Final_Report.txt","a");
    fprintf(graph,"%s","\n\n");

    //City Coordinates
    grid[Y+LT][X+LO] = 'X';

    //Print the Plot
    int x,y = 0;
    for(y=0; y<HEIGHT; y++){
        printf("\t\t\t\t");
        fprintf(graph,"%s","\t\t\t\t");

        for(x=0; x<WIDTH; x++){
            printf("%c",grid[y][x]);
            fprintf(graph,"%c",grid[y][x]);
        }
        printf("\n");
        fprintf(graph,"%c",'\n');
    }

    fclose(graph);
    printf("\n\n\t\t\t\t >>>Indicated by X is the Observer Location\n");
    printf("\n\t\t\t\t >>>Orbit for 30 minutes before and after the input time are shown here.\n");
}

int plot(int x, int y){
    if(x>XMAX || x<XMIN || y>YMAX || y<YMIN){
        return -1;
    }
    else{
        //Mark plot coordinates
        grid[Y+y][X+x] = '*';
        return 1;
    }
}
//---------------------------------------------------------------------------------------------------------------------------------------------
//_______________________________________________________________________________________________________________________________________________________
