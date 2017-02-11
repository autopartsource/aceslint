/*
Compile with mysql support
gcc -o aceslint `xml2-config --cflags` aceslint.c `xml2-config --libs` -L/usr/lib/mysql -lmysqlclient -lz -DWITH_MYSQL

compile without mysql support - vcdb features will be disabled
gcc -o aceslint `xml2-config --cflags` aceslint.c `xml2-config --libs` -L/usr/lib/mysql -lmysqlclient -lz 
*/
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>
#include <ctype.h>
#ifdef WITH_MYSQL
 #include <mysql/mysql.h>
#endif


#ifdef WITH_MYSQL
 MYSQL dbVCDB;
 MYSQL_ROW dbVCDBRow;
 MYSQL_RES *dbVCDBRecset;
 char sql_command[4096]= "";
#endif

struct ACESapp
{
        char action;
        int id;
        int basevid;
        char part[32];
        char notes[512];
        int position;
        int parttype;
        int qty;
	char mfrlabel[64];
	char asset[16];
	int attributeCount;
	char attributeNames[8][32];
	int attributeValues[8];
};



void sprintNiceQualifiers(char *target, char *errors, struct ACESapp *app);
void sprintAttributeSQL(char *target, char *name, int value);
void sprintAttributeWhere(char *target, char *name, int value);
void sprintSQLclausesForApp(char *fromClause, char *whereClause, struct ACESapp *app);
int systemGroupOfAttribute(char *attr);
int appSortCompare(const void *a, const void *b);

int main(int arg_count, char *args[])
{
	char version[8]="0.1.0";
	int i,j,k;
	int subfield_count;
	char strTemp[4096];
	char *charTempPtrA;
	char *charTempPtrB;
	char input_file_name[256]="";
	char item_translation_filename[256]="";
	FILE *item_translation_fileptr=NULL;
	char database_name[256] = "";
	char database_host[256] = "localhost";
	char database_user[64] = "";
	char database_pass[64]= "";
	int database_support=0;
	int database_used=0;
	int verbosity=1;
	int printed_header=0;
	int extract_items=0;
	int extract_assets=0;
	int flatten=0;
	int ignore_na=0;
	int filterfromyear=0;
	int filtertoyear=9999;
	int include_app;
	int makeids_list[512];
	int makeids_list_count=0;
	int makename_filter_mode=0; //=0 no filteing, 1=keep apps who's makes appear in list, 2=drop apps who's makes appear in list

	char makeName[255]="";
	char modelName[255]="";
	int year=0;


	int parttypeids_list[32];
	int parttypeids_list_count=0;
	int parttypeids_filter_mode=0; //=0 no filteing, 1=keep apps who's parttype id's appear in list, 2=drop apps who's parttype id's appear in list


#ifdef WITH_MYSQL
	database_support=1;
#endif


        if(arg_count == 1)
        {// print usage 

		if(database_support)
		{
			printf("\n\n\tusage: aceslint inputfilename [-v <number>] [-d <database name> [-h <database host> -u <datebase user> -p <datebase password>]]\n\n");
		}
		else
		{
			printf("\n\n\tusage: aceslint inputfilename\n\n");
		}

		if(database_support){printf("\tcompiled with mysql database support\n");}else{printf("\tNOT compiled with database support\n");}
		printf("\tversion 2/7/2017\n\n");
		exit(1);
        }

	strcpy(input_file_name,args[1]);

	for(i=1;i<=arg_count-1;i++)
	{
		if(strcmp(args[i],"-d")==0 && i<(arg_count-1)){strcpy(database_name,args[i + 1]); database_used=1;}
		if(strcmp(args[i],"-h")==0 && i<(arg_count-1)){strcpy(database_host,args[i + 1]);}
		if(strcmp(args[i],"-u")==0 && i<(arg_count-1)){strcpy(database_user,args[i + 1]);}
		if(strcmp(args[i],"-p")==0 && i<(arg_count-1)){strcpy(database_pass,args[i + 1]);}
		if(strcmp(args[i],"-v")==0 && i<(arg_count-1)){verbosity=atoi(args[i + 1]);}
		if(strcmp(args[i],"--ignorenaitems")==0){ignore_na=1;}
		if(strcmp(args[i],"--filterbyyears")==0 && i<(arg_count-2)){filterfromyear=atoi(args[i + 1]); filtertoyear=atoi(args[i + 2]);}

		if((strcmp(args[i],"--includemakeids")==0 || strcmp(args[i],"--excludemakeids")==0) && i<(arg_count-1))
		{
			if(strlen(args[i + 1])<1023)
			{
				strcpy(strTemp,args[i + 1]);
				charTempPtrA=strTemp; charTempPtrB=strTemp;
				while (charTempPtrA != NULL)
				{
					strsep(&charTempPtrB, ",");
					if(strlen(charTempPtrA)>6){printf("makeid (%s) was too long - The limit is 6 characters\n",charTempPtrA); exit(1);}
					makeids_list[makeids_list_count]=atoi(charTempPtrA); 
					if(makeids_list[makeids_list_count]<1 || makeids_list[makeids_list_count]>999999){printf("makeid (%s) was out of range - 0-99999\n",charTempPtrA); exit(1);}
					makeids_list_count++;
					charTempPtrA = charTempPtrB;
				}
			}
			else{printf("makenames list was too long\n"); exit(1);}
			if(strcmp(args[i],"--includemakeids")==0){makename_filter_mode=1;}
			if(strcmp(args[i],"--excludemakeids")==0){makename_filter_mode=2;}
		}

		if(strcmp(args[i],"--extractitems")==0){verbosity=0; extract_items=1;}
		if(strcmp(args[i],"--extractassets")==0){verbosity=0; extract_assets=1;}
		if(strcmp(args[i],"--flattenmethod")==0 && i<(arg_count-1)){flatten=atoi(args[i + 1]);}
		if(strcmp(args[i],"--itemtranslationfile")==0 && i<(arg_count-1)){strcpy(item_translation_filename,args[i + 1]);}
	}



//	for(j=0; j<=makeids_list_count-1;j++){printf("%d - *%d*\n",makename_filter_mode,makeids_list[j]);}


	char xmlPathString[256]="/ACES/App";
	xmlChar *xmlCharPtr =NULL;
	xmlDoc *doc = NULL;
	xmlNodePtr cur = NULL;
	xmlNodeSetPtr nodesetHeader;
	xmlNodeSetPtr nodesetApps;
	xmlXPathContextPtr xmlContext;
	xmlXPathObjectPtr xmlResult;

	struct ACESapp *apps[400000];
	int apps_count = 0;
	struct ACESapp tempApp;

	char itemList[20000][16];
	int itemListCount=0;

	char assetList[20000][16];
	int assetListCount=0;

	char document_title[256]="";
	char vcdb_version[32]="";
	char pcdb_version[32]="";
	char qdb_version[32]="";

	char attributeStrA[4096];
	char attributeStrB[4096];
	int maxNoteSize=0;

	int invalid_count=0;
	int found;

	char itemTranslations[20000][2][32];
	int itemTranslationsCount=0;
	int field_index=0;
	int input_byte;

	if(item_translation_filename[0])
	{// translation filename was provied
		item_translation_fileptr = fopen(item_translation_filename,"r"); if(item_translation_fileptr == (FILE *)0){ printf("Error opening item translation file\n"); exit(1);}
		field_index = 0; strTemp[0]=0; j=0;
		while((input_byte = fgetc(item_translation_fileptr)) != EOF)
		{
			if(input_byte == 13){continue;}			// ignore CR's
			if(input_byte == 9)
			{// this byte was a field delimiter - advance field index and grab next byte
				if(field_index<2 && j<31)
				{// only copy the field if we are in the first two columns and the accumulated string is not too long
					strcpy(itemTranslations[itemTranslationsCount][field_index],strTemp);
				}
				strTemp[0]=0; j=0; field_index++; continue;
			}
			if(input_byte == 10)
		        {// just read in a whole row
				if(field_index<2 && strlen(strTemp)<32)
				{// only copy the field if we are in the first two columns and the accumulated string is not too long
					strcpy(itemTranslations[itemTranslationsCount][field_index],strTemp);
					itemTranslationsCount++;
				}
				strTemp[0]=0; j=0; field_index = 0; continue;
			}
			if(j>30){printf("Item translation file contains a field that is too long (on line# %d)\n",itemTranslationsCount+1); exit(1);}
			strTemp[j] = input_byte;
			strTemp[j+1] = 0; // tack on a null-terminator after the current byte
			j++;
		}
		if(field_index==1 && j<31)
		{// there was no LF at the end of last line - consume what's still in the temp str
			strTemp[j+1] = 0; // tack on a null-terminator after the current byte
			itemTranslationsCount++;
		}
		fclose(item_translation_fileptr);
		//for(i=0;i<=itemTranslationsCount-1;i++){printf("*%s*%s*\n",itemTranslations[i][0],itemTranslations[i][1]);}
	}
	if(verbosity>0 && itemTranslationsCount){printf("%d items in translations table\n",itemTranslationsCount);}

	//initialize mysql client - for VCDB connection
#ifdef WITH_MYSQL
	if(database_used)
	{
		if(!mysql_init(&dbVCDB)){printf("mysql client (VCDB) didn't initialize\n"); exit(1);}
		if(!mysql_real_connect(&dbVCDB,database_host,database_user,database_pass,database_name,0,NULL,0)){printf("database connection (VCdb) not established\n"); exit(1);}
	}
#endif

 	doc = xmlReadFile(input_file_name, NULL, 0); if (doc == NULL) {printf("error: could not parse file %s\n", input_file_name);}
	strcpy(xmlPathString,"/ACES/Header"); // extract the vcdbversion from the header section 
	xmlContext = xmlXPathNewContext(doc);
	xmlResult = xmlXPathEvalExpression(BAD_CAST xmlPathString, xmlContext);
	xmlXPathFreeContext(xmlContext);
 	nodesetHeader = xmlResult->nodesetval;
	cur = nodesetHeader->nodeTab[0]->xmlChildrenNode;
	while (cur != NULL)
	{
		if((!xmlStrcmp(cur->name, (const xmlChar *)"DocumentTitle"))){xmlCharPtr=xmlNodeGetContent(cur); strcpy(document_title,(char *)xmlCharPtr); xmlFree(xmlCharPtr);}
		if((!xmlStrcmp(cur->name, (const xmlChar *)"VcdbVersionDate"))){xmlCharPtr=xmlNodeGetContent(cur); strcpy(vcdb_version,(char *)xmlCharPtr); xmlFree(xmlCharPtr);}
		if((!xmlStrcmp(cur->name, (const xmlChar *)"PcdbVersionDate"))){xmlCharPtr=xmlNodeGetContent(cur); strcpy(pcdb_version,(char *)xmlCharPtr); xmlFree(xmlCharPtr);}
		if((!xmlStrcmp(cur->name, (const xmlChar *)"QdbVersionDate"))){xmlCharPtr=xmlNodeGetContent(cur); strcpy(qdb_version,(char *)xmlCharPtr); xmlFree(xmlCharPtr);}
		cur = cur->next;
	}
	xmlXPathFreeObject(xmlResult);
	printf("Title:%s\nVcdbVersionDate:%s\n",document_title,vcdb_version);

	strcpy(xmlPathString,"/ACES/App");
	xmlContext = xmlXPathNewContext(doc);
	xmlResult = xmlXPathEvalExpression(BAD_CAST xmlPathString, xmlContext);
	xmlXPathFreeContext(xmlContext);
 	nodesetApps = xmlResult->nodesetval;

	for (i=0; i < nodesetApps->nodeNr; i++)
	{
		// extract xml tag data from thei app node into temp app structure - we may decide to drop this app later - no need to malloc space for it yet.
		tempApp.action=' ';
		tempApp.id=-1;
		tempApp.basevid=-1;
		tempApp.part[0]=0;
		tempApp.mfrlabel[0]=0;
		tempApp.asset[0]=0;
		tempApp.notes[0]=0;
		tempApp.parttype=-1;
		tempApp.position=0;
		tempApp.qty=0;
		tempApp.attributeCount=0;

		xmlCharPtr=xmlGetProp(nodesetApps->nodeTab[i],(const xmlChar *)"id"); tempApp.id=atoi((char *)xmlCharPtr); xmlFree(xmlCharPtr);
		xmlCharPtr=xmlGetProp(nodesetApps->nodeTab[i],(const xmlChar *)"action"); tempApp.action=xmlCharPtr[0]; xmlFree(xmlCharPtr);

		cur = nodesetApps->nodeTab[i]->xmlChildrenNode;
		while (cur != NULL)
		{
			if ((!xmlStrcmp(cur->name, (const xmlChar *)"BaseVehicle"))){xmlCharPtr=xmlGetProp(cur,(const xmlChar *)"id"); tempApp.basevid=atoi((char *)xmlCharPtr); xmlFree(xmlCharPtr);}
			if ((!xmlStrcmp(cur->name, (const xmlChar *)"Position"))){xmlCharPtr=xmlGetProp(cur,(const xmlChar *)"id"); tempApp.position=atoi((char *)xmlCharPtr); xmlFree(xmlCharPtr);}
			if ((!xmlStrcmp(cur->name, (const xmlChar *)"PartType"))){xmlCharPtr=xmlGetProp(cur,(const xmlChar *)"id"); tempApp.parttype=atoi((char *)xmlCharPtr); xmlFree(xmlCharPtr);}
			if ((!xmlStrcmp(cur->name, (const xmlChar *)"Qty"))){xmlCharPtr=xmlNodeGetContent(cur); tempApp.qty=atoi((char *)xmlCharPtr); xmlFree(xmlCharPtr);}
			if ((!xmlStrcmp(cur->name, (const xmlChar *)"Part"))){xmlCharPtr=xmlNodeGetContent(cur); strcpy(tempApp.part,(char *)xmlCharPtr); xmlFree(xmlCharPtr);}

			//------------    optional tags (qualifiers) ---------------------
			if ((!xmlStrcmp(cur->name, (const xmlChar *)"MfrLabel"))){xmlCharPtr=xmlNodeGetContent(cur); strcpy(tempApp.mfrlabel,(char *)xmlCharPtr); xmlFree(xmlCharPtr);}
			if ((!xmlStrcmp(cur->name, (const xmlChar *)"AssetName"))){xmlCharPtr=xmlNodeGetContent(cur); strcpy(tempApp.asset,(char *)xmlCharPtr); xmlFree(xmlCharPtr);}

			if(!xmlStrcmp(cur->name, (const xmlChar *)"Note"))
			{
				if(strlen((char *)xmlCharPtr)>maxNoteSize){maxNoteSize=strlen((char *)xmlCharPtr);}
				xmlCharPtr=xmlNodeGetContent(cur); strcat(tempApp.notes,(char *)xmlCharPtr); strcat(tempApp.notes,"; "); xmlFree(xmlCharPtr);
			}
			if(!xmlStrcmp(cur->name, (const xmlChar *)"SubModel"))
			{
				xmlCharPtr=xmlGetProp(cur,(const xmlChar *)"id"); tempApp.attributeValues[tempApp.attributeCount]=atoi((char *)xmlCharPtr); xmlFree(xmlCharPtr);
				strcpy(tempApp.attributeNames[tempApp.attributeCount],"SubModel"); tempApp.attributeCount++;
			}
			if(!xmlStrcmp(cur->name, (const xmlChar *)"Region"))
			{
				xmlCharPtr=xmlGetProp(cur,(const xmlChar *)"id"); tempApp.attributeValues[tempApp.attributeCount]=atoi((char *)xmlCharPtr); xmlFree(xmlCharPtr);
				strcpy(tempApp.attributeNames[tempApp.attributeCount],"Region"); tempApp.attributeCount++;
			}
			if(!xmlStrcmp(cur->name, (const xmlChar *)"DriveType"))
			{
				xmlCharPtr=xmlGetProp(cur,(const xmlChar *)"id"); tempApp.attributeValues[tempApp.attributeCount]=atoi((char *)xmlCharPtr); xmlFree(xmlCharPtr);
				strcpy(tempApp.attributeNames[tempApp.attributeCount],"DriveType"); tempApp.attributeCount++;
			}
			if(!xmlStrcmp(cur->name, (const xmlChar *)"FrontBrakeType"))
			{
				xmlCharPtr=xmlGetProp(cur,(const xmlChar *)"id"); tempApp.attributeValues[tempApp.attributeCount]=atoi((char *)xmlCharPtr); xmlFree(xmlCharPtr);
				strcpy(tempApp.attributeNames[tempApp.attributeCount],"FrontBrakeType"); tempApp.attributeCount++;
			}
			if(!xmlStrcmp(cur->name, (const xmlChar *)"RearBrakeType"))
			{
				xmlCharPtr=xmlGetProp(cur,(const xmlChar *)"id"); tempApp.attributeValues[tempApp.attributeCount]=atoi((char *)xmlCharPtr); xmlFree(xmlCharPtr);
				strcpy(tempApp.attributeNames[tempApp.attributeCount],"RearBrakeType"); tempApp.attributeCount++;
			}
			if(!xmlStrcmp(cur->name, (const xmlChar *)"BrakeABS"))
			{
				xmlCharPtr=xmlGetProp(cur,(const xmlChar *)"id"); tempApp.attributeValues[tempApp.attributeCount]=atoi((char *)xmlCharPtr); xmlFree(xmlCharPtr);
				strcpy(tempApp.attributeNames[tempApp.attributeCount],"BrakeABS"); tempApp.attributeCount++;
			}
			if(!xmlStrcmp(cur->name, (const xmlChar *)"BrakeSystem"))
			{
				xmlCharPtr=xmlGetProp(cur,(const xmlChar *)"id"); tempApp.attributeValues[tempApp.attributeCount]=atoi((char *)xmlCharPtr); xmlFree(xmlCharPtr);
				strcpy(tempApp.attributeNames[tempApp.attributeCount],"BrakeSystem"); tempApp.attributeCount++;
			}
			if(!xmlStrcmp(cur->name, (const xmlChar *)"EngineBase"))
			{
				xmlCharPtr=xmlGetProp(cur,(const xmlChar *)"id"); tempApp.attributeValues[tempApp.attributeCount]=atoi((char *)xmlCharPtr); xmlFree(xmlCharPtr);
				strcpy(tempApp.attributeNames[tempApp.attributeCount],"EngineBase"); tempApp.attributeCount++;
			}
			if(!xmlStrcmp(cur->name, (const xmlChar *)"EngineDesignation"))
			{
				xmlCharPtr=xmlGetProp(cur,(const xmlChar *)"id"); tempApp.attributeValues[tempApp.attributeCount]=atoi((char *)xmlCharPtr); xmlFree(xmlCharPtr);
				strcpy(tempApp.attributeNames[tempApp.attributeCount],"EngineDesignation"); tempApp.attributeCount++;
			}
			if(!xmlStrcmp(cur->name, (const xmlChar *)"EngineVIN"))
			{
				xmlCharPtr=xmlGetProp(cur,(const xmlChar *)"id"); tempApp.attributeValues[tempApp.attributeCount]=atoi((char *)xmlCharPtr); xmlFree(xmlCharPtr);
				strcpy(tempApp.attributeNames[tempApp.attributeCount],"EngineVIN"); tempApp.attributeCount++;
			}
			if(!xmlStrcmp(cur->name, (const xmlChar *)"EngineVersion"))
			{
				xmlCharPtr=xmlGetProp(cur,(const xmlChar *)"id"); tempApp.attributeValues[tempApp.attributeCount]=atoi((char *)xmlCharPtr); xmlFree(xmlCharPtr);
				strcpy(tempApp.attributeNames[tempApp.attributeCount],"EngineVersion"); tempApp.attributeCount++;
			}
			if(!xmlStrcmp(cur->name, (const xmlChar *)"EngineMfr"))
			{
				xmlCharPtr=xmlGetProp(cur,(const xmlChar *)"id"); tempApp.attributeValues[tempApp.attributeCount]=atoi((char *)xmlCharPtr); xmlFree(xmlCharPtr);
				strcpy(tempApp.attributeNames[tempApp.attributeCount],"EngineMfr"); tempApp.attributeCount++;
			}
			if(!xmlStrcmp(cur->name, (const xmlChar *)"FuelDeliveryType"))
			{
				xmlCharPtr=xmlGetProp(cur,(const xmlChar *)"id"); tempApp.attributeValues[tempApp.attributeCount]=atoi((char *)xmlCharPtr); xmlFree(xmlCharPtr);
				strcpy(tempApp.attributeNames[tempApp.attributeCount],"FuelDeliveryType"); tempApp.attributeCount++;
			}
			if(!xmlStrcmp(cur->name, (const xmlChar *)"Aspiration"))
			{
				xmlCharPtr=xmlGetProp(cur,(const xmlChar *)"id"); tempApp.attributeValues[tempApp.attributeCount]=atoi((char *)xmlCharPtr); xmlFree(xmlCharPtr);
				strcpy(tempApp.attributeNames[tempApp.attributeCount],"Aspiration"); tempApp.attributeCount++;
			}
			if(!xmlStrcmp(cur->name, (const xmlChar *)"CylinderHeadType"))
			{
				xmlCharPtr=xmlGetProp(cur,(const xmlChar *)"id"); tempApp.attributeValues[tempApp.attributeCount]=atoi((char *)xmlCharPtr); xmlFree(xmlCharPtr);
				strcpy(tempApp.attributeNames[tempApp.attributeCount],"CylinderHeadType"); tempApp.attributeCount++;
			}
			if(!xmlStrcmp(cur->name, (const xmlChar *)"FuelType"))
			{
				xmlCharPtr=xmlGetProp(cur,(const xmlChar *)"id"); tempApp.attributeValues[tempApp.attributeCount]=atoi((char *)xmlCharPtr); xmlFree(xmlCharPtr);
				strcpy(tempApp.attributeNames[tempApp.attributeCount],"FuelType"); tempApp.attributeCount++;
			}
			if(!xmlStrcmp(cur->name, (const xmlChar *)"BodyNumDoors"))
			{
				xmlCharPtr=xmlGetProp(cur,(const xmlChar *)"id"); tempApp.attributeValues[tempApp.attributeCount]=atoi((char *)xmlCharPtr); xmlFree(xmlCharPtr);
				strcpy(tempApp.attributeNames[tempApp.attributeCount],"BodyNumDoors"); tempApp.attributeCount++;
			}
			if(!xmlStrcmp(cur->name, (const xmlChar *)"BodyType"))
			{
				xmlCharPtr=xmlGetProp(cur,(const xmlChar *)"id"); tempApp.attributeValues[tempApp.attributeCount]=atoi((char *)xmlCharPtr); xmlFree(xmlCharPtr);
				strcpy(tempApp.attributeNames[tempApp.attributeCount],"BodyType"); tempApp.attributeCount++;
			}
			if(!xmlStrcmp(cur->name, (const xmlChar *)"MfrBodyCode"))
			{
				xmlCharPtr=xmlGetProp(cur,(const xmlChar *)"id"); tempApp.attributeValues[tempApp.attributeCount]=atoi((char *)xmlCharPtr); xmlFree(xmlCharPtr);
				strcpy(tempApp.attributeNames[tempApp.attributeCount],"MfrBodyCode"); tempApp.attributeCount++;
			}
			if(!xmlStrcmp(cur->name, (const xmlChar *)"TransmissionControlType"))
			{
				xmlCharPtr=xmlGetProp(cur,(const xmlChar *)"id"); tempApp.attributeValues[tempApp.attributeCount]=atoi((char *)xmlCharPtr); xmlFree(xmlCharPtr);
				strcpy(tempApp.attributeNames[tempApp.attributeCount],"TransmissionControlType"); tempApp.attributeCount++;
			}
			if(!xmlStrcmp(cur->name, (const xmlChar *)"TransmissionNumSpeeds"))
			{
				xmlCharPtr=xmlGetProp(cur,(const xmlChar *)"id"); tempApp.attributeValues[tempApp.attributeCount]=atoi((char *)xmlCharPtr); xmlFree(xmlCharPtr);
				strcpy(tempApp.attributeNames[tempApp.attributeCount],"TransmissionNumSpeeds"); tempApp.attributeCount++;
			}
			if(!xmlStrcmp(cur->name, (const xmlChar *)"TransmissionType"))
			{
				xmlCharPtr=xmlGetProp(cur,(const xmlChar *)"id"); tempApp.attributeValues[tempApp.attributeCount]=atoi((char *)xmlCharPtr); xmlFree(xmlCharPtr);
				strcpy(tempApp.attributeNames[tempApp.attributeCount],"TransmissionType"); tempApp.attributeCount++;
			}
			if(!xmlStrcmp(cur->name, (const xmlChar *)"TransmissionMfr"))
			{
				xmlCharPtr=xmlGetProp(cur,(const xmlChar *)"id"); tempApp.attributeValues[tempApp.attributeCount]=atoi((char *)xmlCharPtr); xmlFree(xmlCharPtr);
				strcpy(tempApp.attributeNames[tempApp.attributeCount],"TransmissionMfr"); tempApp.attributeCount++;
			}
			if(!xmlStrcmp(cur->name, (const xmlChar *)"TransmissionMfrCode"))
			{
				xmlCharPtr=xmlGetProp(cur,(const xmlChar *)"id"); tempApp.attributeValues[tempApp.attributeCount]=atoi((char *)xmlCharPtr); xmlFree(xmlCharPtr);
				strcpy(tempApp.attributeNames[tempApp.attributeCount],"TransmissionMfrCode"); tempApp.attributeCount++;
			}
			if(!xmlStrcmp(cur->name, (const xmlChar *)"SteeringType"))
			{
				xmlCharPtr=xmlGetProp(cur,(const xmlChar *)"id"); tempApp.attributeValues[tempApp.attributeCount]=atoi((char *)xmlCharPtr); xmlFree(xmlCharPtr);
				strcpy(tempApp.attributeNames[tempApp.attributeCount],"SteeringType"); tempApp.attributeCount++;
			}
			if(!xmlStrcmp(cur->name, (const xmlChar *)"SteeringSystem"))
			{
				xmlCharPtr=xmlGetProp(cur,(const xmlChar *)"id"); tempApp.attributeValues[tempApp.attributeCount]=atoi((char *)xmlCharPtr); xmlFree(xmlCharPtr);
				strcpy(tempApp.attributeNames[tempApp.attributeCount],"SteeringSystem"); tempApp.attributeCount++;
			}
			if(!xmlStrcmp(cur->name, (const xmlChar *)"FrontSpringType"))
			{
				xmlCharPtr=xmlGetProp(cur,(const xmlChar *)"id"); tempApp.attributeValues[tempApp.attributeCount]=atoi((char *)xmlCharPtr); xmlFree(xmlCharPtr);
				strcpy(tempApp.attributeNames[tempApp.attributeCount],"FrontSpringType"); tempApp.attributeCount++;
			}
			if(!xmlStrcmp(cur->name, (const xmlChar *)"RearSpringType"))
			{
				xmlCharPtr=xmlGetProp(cur,(const xmlChar *)"id"); tempApp.attributeValues[tempApp.attributeCount]=atoi((char *)xmlCharPtr); xmlFree(xmlCharPtr);
				strcpy(tempApp.attributeNames[tempApp.attributeCount],"RearSpringType"); tempApp.attributeCount++;
			}
			if(!xmlStrcmp(cur->name, (const xmlChar *)"WheelBase"))
			{
				xmlCharPtr=xmlGetProp(cur,(const xmlChar *)"id"); tempApp.attributeValues[tempApp.attributeCount]=atoi((char *)xmlCharPtr); xmlFree(xmlCharPtr);
				strcpy(tempApp.attributeNames[tempApp.attributeCount],"WheelBase"); tempApp.attributeCount++;
			}
			if(!xmlStrcmp(cur->name, (const xmlChar *)"BedType"))
			{
				xmlCharPtr=xmlGetProp(cur,(const xmlChar *)"id"); tempApp.attributeValues[tempApp.attributeCount]=atoi((char *)xmlCharPtr); xmlFree(xmlCharPtr);
				strcpy(tempApp.attributeNames[tempApp.attributeCount],"BedType"); tempApp.attributeCount++;
			}
			if(!xmlStrcmp(cur->name, (const xmlChar *)"BedLength"))
			{
				xmlCharPtr=xmlGetProp(cur,(const xmlChar *)"id"); tempApp.attributeValues[tempApp.attributeCount]=atoi((char *)xmlCharPtr); xmlFree(xmlCharPtr);
				strcpy(tempApp.attributeNames[tempApp.attributeCount],"BedLength"); tempApp.attributeCount++;
			}
			cur = cur->next;
		}

		include_app=1;
		while(1)
		{// evaluate all the reasons to exclude this app

			if(ignore_na && apps[apps_count]->part[0]=='N' && apps[apps_count]->part[1]=='A' && apps[apps_count]->part[2]==0)
			{// this app's part number is "NA" and we have elected to ignore NA's. free the malloc'd memory for this app and don't advance the app count
				include_app=0; break;
			}

			if(itemTranslationsCount)
			{// there is a translation list in play 
				found=0;
				for(j=0; j<=itemTranslationsCount-1; j++)
				{
					if(strcmp(tempApp.part,itemTranslations[j][0])==0){found=1; break;}
				}

				if(found)
				{// found this app's item in the translation list 
					strcpy(tempApp.part,itemTranslations[j][1]);
				}
				else
				{// no translation was found for this app's part number - delete (free) the app from the array, and don't inc the apps count
					include_app=0; break;
				}
			}

#ifdef WITH_MYSQL
			if(database_used && (filterfromyear>0 || filtertoyear<9999))
			{// modelyear filtration is in play

				sprintf(sql_command,"select makeid,modelid,yearid from basevehicle where basevehicleid=%d;",tempApp.basevid); if(mysql_query(&dbVCDB,sql_command)){printf("\nSQL Error\n%s\n",sql_command);exit(1);} dbVCDBRecset = mysql_store_result(&dbVCDB);
				if(dbVCDBRow = mysql_fetch_row(dbVCDBRecset))
				{
					strcpy(makeName,dbVCDBRow[0]); strcpy(modelName,dbVCDBRow[1]); year=atoi(dbVCDBRow[2]);
					if(year<filterfromyear || year>filtertoyear){include_app=0;}
				}
				mysql_free_result(dbVCDBRecset); break;
			}

			if(database_used && makename_filter_mode)
			{// test this app against make include/exclude list
				if(makename_filter_mode==1)
				{// list provided is an include list 
					include_app=0;
					sprintf(sql_command,"select makeid from basevehicle where basevehicleid=%d;",tempApp.basevid); if(mysql_query(&dbVCDB,sql_command)){printf("\nSQL Error\n%s\n",sql_command);exit(1);} dbVCDBRecset = mysql_store_result(&dbVCDB);
					if(dbVCDBRow = mysql_fetch_row(dbVCDBRecset))
					{
						for(j=0; j<=makeids_list_count-1;j++){if(makeids_list[j]==atoi(dbVCDBRow[0])){include_app=1; break;}}
					}mysql_free_result(dbVCDBRecset); break;
				}
				if(makename_filter_mode==2)
				{// list provided is an exclude list 
					sprintf(sql_command,"select makeid from basevehicle where basevehicleid=%d;",tempApp.basevid); if(mysql_query(&dbVCDB,sql_command)){printf("\nSQL Error\n%s\n",sql_command);exit(1);} dbVCDBRecset = mysql_store_result(&dbVCDB);
					if(dbVCDBRow = mysql_fetch_row(dbVCDBRecset))
					{
						for(j=0; j<=makeids_list_count-1;j++){if(makeids_list[j]==atoi(dbVCDBRow[0])){include_app=0; break;}}
					}mysql_free_result(dbVCDBRecset); break;
				}
			}
#endif

			break;
		}

		if(include_app)
		{// we did not find a reason to exclude this app. allocate memory and add its pointer to the array
			apps[apps_count] = (struct ACESapp *) malloc(sizeof(struct ACESapp));
			apps[apps_count]->action=tempApp.action;
			apps[apps_count]->id=tempApp.id;
			apps[apps_count]->basevid=tempApp.basevid;
			strcpy(apps[apps_count]->part,tempApp.part);
			strcpy(apps[apps_count]->mfrlabel,tempApp.mfrlabel);
			strcpy(apps[apps_count]->asset,tempApp.asset);
			strcpy(apps[apps_count]->notes,tempApp.notes);
			apps[apps_count]->parttype=tempApp.parttype;
			apps[apps_count]->position=tempApp.position;
			apps[apps_count]->qty=tempApp.qty;
			apps[apps_count]->attributeCount=tempApp.attributeCount;
			for(j=0; j<=tempApp.attributeCount-1; j++){strcpy(apps[apps_count]->attributeNames[j],tempApp.attributeNames[j]); apps[apps_count]->attributeValues[j]=tempApp.attributeValues[j];}
			apps_count++;
		}

	}

	// free xml nodeset - all data that we care about is now in the apps[] array 
	xmlXPathFreeObject(xmlResult);
	if(verbosity>0){printf("Application count:%d\n",apps_count);}

	qsort(apps, apps_count, sizeof(struct ACESapp *), appSortCompare);

#ifdef WITH_MYSQL
	if(database_used)
	{
		//check validity of basevids 
		if(verbosity>2){printf("checking for valid basevids\n");}
		invalid_count=0; printed_header=0;
		for(i=0;i<=apps_count-1;i++)
		{
			attributeStrA[0]=0;for(j=0; j<=apps[i]->attributeCount-1; j++){snprintf(strTemp,1024,"%s:%d;",apps[i]->attributeNames[j],apps[i]->attributeValues[j]); strcat(attributeStrA,strTemp);} strcat(attributeStrA,apps[i]->notes);
			sprintf(sql_command,"select makeid from basevehicle where basevehicleid=%d;",apps[i]->basevid); if(mysql_query(&dbVCDB,sql_command)){printf("\nSQL Error\n%s\n",sql_command);exit(1);} dbVCDBRecset = mysql_store_result(&dbVCDB);
			dbVCDBRow = mysql_fetch_row(dbVCDBRecset);
			if(dbVCDBRow==NULL)
			{// basevehicleid was not found in vcdb
				if(!printed_header && verbosity>2){printf("App Id\tBase vid\tPart Type\tPosition\tPart\tQualifiers\n"); printed_header=1;}
				if(verbosity>2){printf("%d\t%d\t%d\t%d\t%s\t%s\n",apps[i]->id,apps[i]->basevid,apps[i]->parttype,apps[i]->position,apps[i]->part,attributeStrA);}
				invalid_count++;
			}
			mysql_free_result(dbVCDBRecset);
		}
		if(verbosity>0){printf("Invalid basevids:%d\n",invalid_count);}

		//check validity of coded attributes 
		if(verbosity>2){printf("checking for valid attribute ids\n");}
		invalid_count=0; printed_header=0;
		for(i=0;i<=apps_count-1;i++)
		{
			sprintNiceQualifiers(attributeStrA, attributeStrB, apps[i]);
			if(attributeStrB[0]!=0)
			{
				if(!printed_header && verbosity>2){printf("App Id\tBase vid\tPart Type\tPosition\tPart\tQualifiers\tErrors\n"); printed_header=1;}
				if(verbosity>2){printf("%d\t%d\t%d\t%d\t%s\t%s\t%s\n",apps[i]->id,apps[i]->basevid,apps[i]->parttype,apps[i]->position,apps[i]->part,attributeStrA,attributeStrB);}
				invalid_count++;
			}
		}
		if(verbosity>0){printf("Invalid vcdb codes:%d\n",invalid_count);}

		//check validity of coded attributes configurations
		if(verbosity>2){printf("checking for valid vcdb configurtions\n");}
		invalid_count=0; printed_header=0;
		for(i=0;i<=apps_count-1;i++)
		{
			sprintSQLclausesForApp(attributeStrA, attributeStrB, apps[i]);
			if(attributeStrA[0]>0)
			{
				sprintf(sql_command,"select vehicle.vehicleid from %s where %s;",attributeStrA, attributeStrB); if(mysql_query(&dbVCDB,sql_command)){printf("\nSQL Error\n%s\n",sql_command);exit(1);} dbVCDBRecset = mysql_store_result(&dbVCDB);
				dbVCDBRow = mysql_fetch_row(dbVCDBRecset);
				mysql_free_result(dbVCDBRecset);
				if(dbVCDBRow==NULL)
				{// combination of attributes was not found in vcdb for basevid
					if(verbosity>2)
					{
						sprintNiceQualifiers(attributeStrA, attributeStrB, apps[i]);
						if(!printed_header && verbosity>2){printf("App Id\tBase vid\tPart Type\tPosition\tPart\tQualifiers\tErrors\n"); printed_header=1;}
						printf("%d\t%d\t%d\t%d\t%s\t%s\t%s\n",apps[i]->id,apps[i]->basevid,apps[i]->parttype,apps[i]->position,apps[i]->part,attributeStrA,attributeStrB);
					}
					invalid_count++;
				}
			}
		}
		if(verbosity>0){printf("invalid vcdb configurations:%d\n",invalid_count);}
	}

#endif

	//check for duplicates
	if(verbosity>2){printf("checking for duplicates\n");}
	invalid_count=0; printed_header=0;
	for(i=0;i<=apps_count-2;i++)
	{
		attributeStrA[0]=0; attributeStrB[0]=0;
		for(j=0; j<=apps[i]->attributeCount-1; j++){snprintf(strTemp,1024,"%s:%d;",apps[i]->attributeNames[j],apps[i]->attributeValues[j]); strcat(attributeStrA,strTemp);} strcat(attributeStrA,apps[i]->notes);
		for(j=0; j<=apps[i+1]->attributeCount-1; j++){snprintf(strTemp,1024,"%s:%d;",apps[i+1]->attributeNames[j],apps[i+1]->attributeValues[j]); strcat(attributeStrB,strTemp);} strcat(attributeStrB,apps[i+1]->notes);
		if(apps[i]->basevid==apps[i+1]->basevid && apps[i]->parttype==apps[i+1]->parttype && apps[i]->position==apps[i+1]->position && strcmp(apps[i]->mfrlabel,apps[i+1]->mfrlabel)==0 && strcmp(apps[i]->part,apps[i+1]->part)==0 && strcmp(attributeStrA,attributeStrB)==0)
		{
#ifdef WITH_MYSQL
			if(verbosity>2)
			{
				if(database_used)
				{
					if(!printed_header){printf("Make\tModel\tYear\tBase vid\tPart Type\tPosition\tPart\tQualifiers\n"); printed_header=1;}
					sprintf(sql_command,"select makename,modelname,yearid from basevehicle,make,model where basevehicle.makeid = make.makeid and basevehicle.modelid = model.modelid and basevehicle.basevehicleid=%d;",apps[i]->basevid); if(mysql_query(&dbVCDB,sql_command)){printf("\nSQL Error\n%s\n",sql_command);exit(1);} dbVCDBRecset = mysql_store_result(&dbVCDB);
					makeName[0]=0; modelName[0]=0; year=0; if((dbVCDBRow = mysql_fetch_row(dbVCDBRecset))){strcpy(makeName,dbVCDBRow[0]); strcpy(modelName,dbVCDBRow[1]); year=atoi(dbVCDBRow[2]);} mysql_free_result(dbVCDBRecset);
					printf("%s\t%s\t%d\t%d\t%d\t%d\t%s\t%s\n",makeName,modelName,year,apps[i]->basevid,apps[i]->parttype,apps[i]->position,apps[i]->part,attributeStrA);
					printf("%s\t%s\t%d\t%d\t%d\t%d\t%s\t%s\n",makeName,modelName,year,apps[i]->basevid,apps[i]->parttype,apps[i]->position,apps[i]->part,attributeStrA);
				}
				else
				{// no database connection was specified 
					if(!printed_header){printf("Base vid\tPart Type\tPosition\tPart\tQualifiers\n"); printed_header=1;}
					printf("%d\t%d\t%d\t%s\t%s\n",apps[i]->basevid,apps[i]->parttype,apps[i]->position,apps[i]->part,attributeStrA);
					printf("%d\t%d\t%d\t%s\t%s\n",apps[i]->basevid,apps[i]->parttype,apps[i]->position,apps[i]->part,attributeStrA);
				}
			}
#else
			if(verbosity>2)
			{
				if(!printed_header){printf("Base vid\tPart Type\tPosition\tPart\tQualifiers\n"); printed_header=1;}
				printf("%d\t%d\t%d\t%s\t%s\n",apps[i]->basevid,apps[i]->parttype,apps[i]->position,apps[i]->part,attributeStrA);
				printf("%d\t%d\t%d\t%s\t%s\n",apps[i]->basevid,apps[i]->parttype,apps[i]->position,apps[i]->part,attributeStrA);
			}
#endif
			invalid_count++;
		}
	}
	if(verbosity>0){printf("Duplicate apps:%d\n",invalid_count);}


	//check for overlaps
	if(verbosity>2){printf("checking for overlaps\n");}
	invalid_count=0; printed_header=0;
	for(i=0;i<=apps_count-2;i++)
	{
		attributeStrA[0]=0; attributeStrB[0]=0;
		for(j=0; j<=apps[i]->attributeCount-1; j++){snprintf(strTemp,1024,"%s:%d;",apps[i]->attributeNames[j],apps[i]->attributeValues[j]); strcat(attributeStrA,strTemp);} strcat(attributeStrA,apps[i]->notes);
		for(j=0; j<=apps[i+1]->attributeCount-1; j++){snprintf(strTemp,1024,"%s:%d;",apps[i+1]->attributeNames[j],apps[i+1]->attributeValues[j]); strcat(attributeStrB,strTemp);} strcat(attributeStrB,apps[i+1]->notes);
		if(apps[i]->basevid==apps[i+1]->basevid && apps[i]->parttype==apps[i+1]->parttype && apps[i]->position==apps[i+1]->position && strcmp(apps[i]->mfrlabel,apps[i+1]->mfrlabel)==0 && strcmp(apps[i]->part,apps[i+1]->part)!=0 && strcmp(attributeStrA,attributeStrB)==0)
		{

#ifdef WITH_MYSQL
			if(verbosity>2)
			{
				if(database_used)
				{
					if(!printed_header){printf("Make\tModel\tYear\tBase vid A\tPart Type\tPosition\tPart A\tQualifiers A\tPart B\tQualifiers B\n"); printed_header=1;}
					sprintf(sql_command,"select makename,modelname,yearid from basevehicle,make,model where basevehicle.makeid = make.makeid and basevehicle.modelid = model.modelid and basevehicle.basevehicleid=%d;",apps[i]->basevid); if(mysql_query(&dbVCDB,sql_command)){printf("\nSQL Error\n%s\n",sql_command);exit(1);} dbVCDBRecset = mysql_store_result(&dbVCDB);
					makeName[0]=0; modelName[0]=0; year=0; if((dbVCDBRow = mysql_fetch_row(dbVCDBRecset))){strcpy(makeName,dbVCDBRow[0]); strcpy(modelName,dbVCDBRow[1]); year=atoi(dbVCDBRow[2]);} mysql_free_result(dbVCDBRecset);
					printf("%s\t%s\t%d\t%d\t%d\t%d\t%s\t%s\t%s\t%s\n",makeName,modelName,year,apps[i]->basevid,apps[i]->parttype,apps[i]->position,apps[i]->part,attributeStrA,apps[i+1]->part,attributeStrB);
				}
				else
				{// no database was specified
					if(!printed_header){printf("Base vid A\tPart Type\tPosition\tPart A\tQualifiers A\tPart B\tQualifiers B\n"); printed_header=1;}
					printf("%d\t%d\t%d\t%s\t%s\t%s\t%s\n",apps[i]->basevid,apps[i]->parttype,apps[i]->position,apps[i]->part,attributeStrA,apps[i+1]->part,attributeStrB);
				}
			}
#else
			if(verbosity>2)
			{
				if(!printed_header){printf("Base vid A\tPart Type\tPosition\tPart A\tQualifiers A\tPart B\tQualifiers B\n"); printed_header=1;}
				printf("%d\t%d\t%d\t%s\t%s\t%s\t%s\n",apps[i]->basevid,apps[i]->parttype,apps[i]->position,apps[i]->part,attributeStrA,apps[i+1]->part,attributeStrB);
			}
#endif
			invalid_count++;
		}
	}
	if(verbosity>0){printf("Overlaps:%d\n",invalid_count);}

	//check for comment-no-comment errors
	if(verbosity>2){printf("checking for CNCs...\n");}
#ifdef WITH_MYSQL
	if(verbosity>2){if(database_used){printf("Make\tModel\tYear\tBase vid A\tPart Type\tPosition\tPart A\tQualifiers A\tPart B\tQualifiers B\n");}else{printf("Base vid A\tPart Type\tPosition\tPart A\tQualifiers A\tPart B\tQualifiers B\n");}}
#else
	if(verbosity>2){printf("Base vid A\tPart Type\tPosition\tPart A\tQualifiers A\tPart B\tQualifiers B\n");}
#endif
	invalid_count=0;
	for(i=0;i<=apps_count-2;i++)
	{
		attributeStrA[0]=0; attributeStrB[0]=0;
		for(j=0; j<=apps[i]->attributeCount-1; j++){snprintf(strTemp,1024,"%s:%d;",apps[i]->attributeNames[j],apps[i]->attributeValues[j]); strcat(attributeStrA,strTemp);} strcat(attributeStrA,apps[i]->notes);
		for(j=0; j<=apps[i+1]->attributeCount-1; j++){snprintf(strTemp,1024,"%s:%d;",apps[i+1]->attributeNames[j],apps[i+1]->attributeValues[j]); strcat(attributeStrB,strTemp);} strcat(attributeStrB,apps[i+1]->notes);
		if(apps[i]->basevid==apps[i+1]->basevid && apps[i]->parttype==apps[i+1]->parttype && apps[i]->position==apps[i+1]->position &&strcmp(apps[i]->mfrlabel,apps[i+1]->mfrlabel)==0 &&((strlen(attributeStrA)==0 && strlen(attributeStrB)!=0)||(strlen(attributeStrA)!=0 && strlen(attributeStrB)==0)))
		{
#ifdef WITH_MYSQL
			if(verbosity>2)
			{
				if(database_used)
				{
					sprintf(sql_command,"select makename,modelname,yearid from basevehicle,make,model where basevehicle.makeid = make.makeid and basevehicle.modelid = model.modelid and basevehicle.basevehicleid=%d;",apps[i]->basevid); if(mysql_query(&dbVCDB,sql_command)){printf("\nSQL Error\n%s\n",sql_command);exit(1);} dbVCDBRecset = mysql_store_result(&dbVCDB);
					makeName[0]=0; modelName[0]=0; year=0; if((dbVCDBRow = mysql_fetch_row(dbVCDBRecset))){strcpy(makeName,dbVCDBRow[0]); strcpy(modelName,dbVCDBRow[1]); year=atoi(dbVCDBRow[2]);} mysql_free_result(dbVCDBRecset);
					printf("%s\t%s\t%d\t%d\t%d\t%d\t%s\t%s\t%s\t%s\n",makeName,modelName,year,apps[i]->basevid,apps[i]->parttype,apps[i]->position,apps[i]->part,attributeStrA,apps[i+1]->part,attributeStrB);
				}
				else
				{// no database was spefified
					printf("%d\t%d\t%d\t%s\t%s\t%s\t%s\n",apps[i]->basevid,apps[i]->parttype,apps[i]->position,apps[i]->part,attributeStrA,apps[i+1]->part,attributeStrB);
				}
			}
#else
			if(verbosity>2){printf("%d\t%d\t%d\t%s\t%s\t%s\t%s\n",apps[i]->basevid,apps[i]->parttype,apps[i]->position,apps[i]->part,attributeStrA,apps[i+1]->part,attributeStrB);}
#endif
			invalid_count++;
		}
	}
	if(verbosity>0){printf("CNC overlaps:%d\n",invalid_count);}

	if(extract_items)
	{
		for(i=0;i<=apps_count-1;i++)
		{
			found=0;
			for(j=0;j<=itemListCount-1;j++)
			{
				if(strcmp(apps[i]->part,itemList[j])==0){found=1; break;}
			}
			if(!found){strcpy(itemList[itemListCount],apps[i]->part); itemListCount++;}
		}
		for(j=0;j<=itemListCount-1;j++){printf("%s\n",itemList[j]);}
	}

	if(extract_assets)
	{
		for(i=0;i<=apps_count-1;i++)
		{
			found=0;
			for(j=0;j<=assetListCount-1;j++)
			{
				if(strcmp(apps[i]->asset,assetList[j])==0){found=1; break;}
			}
			if(!found){strcpy(assetList[assetListCount],apps[i]->asset); assetListCount++;}
		}
		for(j=0;j<=assetListCount-1;j++){printf("%s\n",assetList[j]);}
	}

	if(flatten>0)
	{
		switch(flatten)
		{
			case 1:
			//coded values only (no human -readable)
				printf("basevid\tpart\tparttypeid\tpositionid\tquantity\tqualifers\tnotes\r\n");
				for(i=0;i<=apps_count-1;i++)
				{
					attributeStrA[0]=0; for(j=0; j<=apps[i]->attributeCount-1; j++){sprintf(strTemp,"%s:%d;",apps[i]->attributeNames[j],apps[i]->attributeValues[j]); strcat(attributeStrA,strTemp);}
					printf("%d\t%s\t%d,%d\t%d\t%s\t%s\r\n",apps[i]->basevid,apps[i]->part,apps[i]->parttype,apps[i]->position,apps[i]->qty,attributeStrA,apps[i]->notes);
				}
				break;

			case 2:
			// human-raedable
				if(database_used)
				{
#ifdef WITH_MYSQL
					for(i=0;i<=apps_count-1;i++)
					{
						sprintf(sql_command,"select makename,modelname,yearid from basevehicle,make,model where basevehicle.makeid = make.makeid and basevehicle.modelid = model.modelid and basevehicle.basevehicleid=%d;",apps[i]->basevid); if(mysql_query(&dbVCDB,sql_command)){printf("\nSQL Error\n%s\n",sql_command);exit(1);} dbVCDBRecset = mysql_store_result(&dbVCDB);
						makeName[0]=0; modelName[0]=0; year=0; if((dbVCDBRow = mysql_fetch_row(dbVCDBRecset))){strcpy(makeName,dbVCDBRow[0]); strcpy(modelName,dbVCDBRow[1]); year=atoi(dbVCDBRow[2]);} mysql_free_result(dbVCDBRecset);
						sprintNiceQualifiers(attributeStrA, attributeStrB, apps[i]);
						printf("%s\t%s\t%d\t%d\t%d\t%d\t%s\t%s\t%s\t%s\r\n",makeName,modelName,year,apps[i]->parttype,apps[i]->position,apps[i]->qty,apps[i]->part,attributeStrA,apps[i]->notes,attributeStrB);
					}
#endif
				}
				else
				{// no database avail
					printf("you must specify a VCdb database in order to flatten to human-readable format.\n");
				}
				break;

			default: break;
		}
	}



	xmlFreeDoc(doc);
	xmlCleanupParser();
	// free apps from memory
	for(i=0;i<=apps_count-1;i++){free(apps[i]);} apps_count=0;

}


#ifdef WITH_MYSQL
// build the "from" and "where" sql tables and join clauses for vcdb validation query based on the attributes in the reference app
// the purpose is to tease out a list of attribute names for knowing which tables to validate against. You could simply validate every app against a monolithic all-in-one 
// join of the entire vcdb - this is process-intensive (very slow). If we only include the tables in the join that the app referes-to, the query is faster and more memory effecient.
// 
void sprintSQLclausesForApp(char *fromClause, char *whereClause, struct ACESapp *app)
{
	fromClause[0]=0; whereClause[0]=0;
	if(app->attributeCount==0){return;}

	char strTemp[2048];
	int found;
	int vcdbSystem;
	int vcdbSystems[16];
	int vcdbSystemsCount=0;
	int i,j;

	for(i=0; i<=app->attributeCount-1; i++)
	{
		vcdbSystem=systemGroupOfAttribute(app->attributeNames[i]);
		found=0;
		for(j=0;j<=vcdbSystemsCount-1; j++)
		{
			if(vcdbSystems[j]==vcdbSystem){found=1; break;}
		}
		if(!found)
		{
			vcdbSystems[vcdbSystemsCount]=vcdbSystem; vcdbSystemsCount++;
		}
	}

	strcpy(fromClause,"vehicle,");
	for(i=0; i<=vcdbSystemsCount-1; i++)
	{
		switch(vcdbSystems[i])
		{
                        case 0:
			// vehilce is the only table required for determining region or submodel. fromClause is initialized to "vheicle," already
                        break;

                        case 1:
			strcat(fromClause,"vehicletodrivetype,");
			strcat(whereClause,"vehicle.vehicleid=vehicletodrivetype.vehicleid and ");
                        break;

                        case 2:
			strcat(fromClause,"vehicletobrakeconfig,brakeconfig,");
			strcat(whereClause,"vehicle.vehicleid=vehicletobrakeconfig.vehicleid and vehicletobrakeconfig.brakeconfigid=brakeconfig.brakeconfigid and ");
                        break;

                        case 3:
			strcat(fromClause,"vehicletoengineconfig,engineconfig,valves,enginebase,fueldeliveryconfig,");
			strcat(whereClause,"vehicle.vehicleid = vehicletoengineconfig.vehicleid and vehicletoengineconfig.engineconfigid = engineconfig.engineconfigid and engineconfig.enginebaseid=enginebase.enginebaseid and engineconfig.valvesid=valves.valvesid and engineconfig.fueldeliveryconfigid=fueldeliveryconfig.fueldeliveryconfigid and ");
                        break;

                        case 4:
			strcat(fromClause,"vehicletobodystyleconfig,bodystyleconfig,");
			strcat(whereClause,"vehicle.vehicleid=vehicletobodystyleconfig.vehicleid and vehicletobodystyleconfig.bodystyleconfigid = bodystyleconfig.bodystyleconfigid and ");
                        break;

                        case 5:
			strcat(fromClause,"vehicletomfrbodycode,");
			strcat(whereClause,"vehicle.vehicleid=vehicletomfrbodycode.vehicleid and ");
                        break;

                        case 6:
			strcat(fromClause,"vehicletotransmission,transmission,transmissionbase,");
			strcat(whereClause,"vehicle.vehicleid=vehicletotransmission.vehicleid and vehicletotransmission.transmissionid=transmission.transmissionid and  transmission.transmissionbaseid=transmissionbase.transmissionbaseid and ");
                        break;

                        case 7:
			strcat(fromClause,"vehicletowheelbase,");
			strcat(whereClause,"vehicle.vehicleid=vehicletowheelbase.vehicleid and ");
                        break;

                        case 8:
			strcat(fromClause,"vehicletosteeringconfig,steeringconfig,");
			strcat(whereClause,"vehicle.vehicleid=vehicletosteeringconfig.vehicleid and vehicletosteeringconfig.steeringconfigid=steeringconfig.steeringconfigid and ");
                        break;

                        case 9:
			strcat(fromClause,"vehicletobedconfig,bedconfig,");
			strcat(whereClause,"vehicle.vehicleid=vehicletobedconfig.vehicleid and vehicletobedconfig.bedconfigid=bedconfig.bedconfigid and ");
                        break;

                        case 10:
			strcat(fromClause,"vehicletospringtypeconfig,springtypeconfig,");
			strcat(whereClause,"vehicle.vehicleid=vehicletospringtypeconfig.vehicleid and vehicletospringtypeconfig.springconfigid=springtypeconfig.springtypeconfigid and ");
                        break;

                        case 11:
			strcat(fromClause,"basevehicle,model,");
			strcat(whereClause,"basebehicle.modelid=model.modelid and ");
                        break;

                        case 12:
                        break;

                        default:
                        break;
		}
	}

	for(i=0; i<=app->attributeCount-1; i++)
	{
		sprintAttributeWhere(strTemp, app->attributeNames[i], app->attributeValues[i]);
		strcat(whereClause,strTemp);
	}

	sprintf(strTemp,"vehicle.basevehicleid = %d ",app->basevid);
	strcat(whereClause,strTemp);
	fromClause[strlen(fromClause)-1]=0; //kill the trailing ','
}



void sprintNiceQualifiers(char *target, char *errors, struct ACESapp *app)
{
	int j;
	char strTemp[1024];
	char strErrorsTemp[256]="";
	target[0]=0; // init the target string to null
	errors[0]=0;

	for(j=0; j<=app->attributeCount-1; j++)
	{
		strTemp[0]=0; strErrorsTemp[0]=0;
		sprintAttributeSQL(sql_command,app->attributeNames[j],app->attributeValues[j]);
		if(sql_command[0]!=0)
		{
			if(mysql_query(&dbVCDB,sql_command)){printf("\nSQL Error\n%s\n",sql_command);exit(1);}
			dbVCDBRecset = mysql_store_result(&dbVCDB);
			dbVCDBRow = mysql_fetch_row(dbVCDBRecset);
			if(dbVCDBRow==NULL)
			{// empty result set from query
				sprintf(strTemp,"invalid vcdb code (%s=%d); ",app->attributeNames[j],app->attributeValues[j]);
				strcpy(strErrorsTemp,strTemp);
			}
			else
			{// got result from query
				sprintf(strTemp,"%s; ",dbVCDBRow[0]);
				if(strcmp(app->attributeNames[j],"MfrBodyCode")==0){sprintf(strTemp,"Body code %s; ",dbVCDBRow[0]);}
				if(strcmp(app->attributeNames[j],"BodyNumDoors")==0){sprintf(strTemp,"%s Door; ",dbVCDBRow[0]);}
				if(strcmp(app->attributeNames[j],"EngineBase")==0){sprintf(strTemp,"%s%s %sL; ",dbVCDBRow[4],dbVCDBRow[3],dbVCDBRow[0]);}
				if(strcmp(app->attributeNames[j],"EngineVIN")==0){sprintf(strTemp,"VIN:%s; ",dbVCDBRow[0]);}
				if(strcmp(app->attributeNames[j],"TransmissionMfrCode")==0){sprintf(strTemp,"%s Transmission; ",dbVCDBRow[0]);}
				if(strcmp(app->attributeNames[j],"TransmissionBase")==0){sprintf(strTemp,"%s %s Speed %s; ",dbVCDBRow[0],dbVCDBRow[1],dbVCDBRow[2]);}
				if(strcmp(app->attributeNames[j],"TransmissionControlType")==0){sprintf(strTemp,"%s Transmission; ",dbVCDBRow[0]);}
				if(strcmp(app->attributeNames[j],"TransmissionNumSpeeds")==0){sprintf(strTemp,"%s Speed Transmission; ",dbVCDBRow[0]);}
				if(strcmp(app->attributeNames[j],"TransmissionMfr")==0){sprintf(strTemp,"%s Transmission; ",dbVCDBRow[0]);}
				if(strcmp(app->attributeNames[j],"BedLength")==0){sprintf(strTemp,"%s Inch Bed; ",dbVCDBRow[0]);}
				if(strcmp(app->attributeNames[j],"BedType")==0){sprintf(strTemp,"%s Bed; ",dbVCDBRow[0]);}
				if(strcmp(app->attributeNames[j],"WheelBase")==0){sprintf(strTemp,"%s Inch Wheelbase; ",dbVCDBRow[0]);}
				if(strcmp(app->attributeNames[j],"BrakeSystem")==0){sprintf(strTemp,"%s Brakes; ",dbVCDBRow[0]);}
				if(strcmp(app->attributeNames[j],"FrontBrakeType")==0){sprintf(strTemp,"Front %s; ",dbVCDBRow[0]);}
				if(strcmp(app->attributeNames[j],"RearBrakeType")==0){sprintf(strTemp,"Rear %s; ",dbVCDBRow[0]);}
				if(strcmp(app->attributeNames[j],"FrontSpringType")==0){sprintf(strTemp,"Front %s Suspenssion; ",dbVCDBRow[0]);}
				if(strcmp(app->attributeNames[j],"RearSpringType")==0){sprintf(strTemp,"Rear %s Suspenssion; ",dbVCDBRow[0]);}
				if(strcmp(app->attributeNames[j],"SteeringSystem")==0){sprintf(strTemp,"%s Steering; ",dbVCDBRow[0]);}
				if(strcmp(app->attributeNames[j],"SteeringType")==0){sprintf(strTemp,"%s Steering; ",dbVCDBRow[0]);}
				if(strcmp(app->attributeNames[j],"ValvesPerEngine")==0){sprintf(strTemp,"%s Valve; ",dbVCDBRow[0]);}
			}
			mysql_free_result(dbVCDBRecset);
		}
		else
		{// unknown attribute name - unlikely if this file passed xsd validation 
			sprintf(strTemp,"unknown vehicle attribute (%s=%d); ",app->attributeNames[j],app->attributeValues[j]);
			strcpy(strErrorsTemp,strTemp);
		}
		strcat(target,strTemp); strcat(errors,strErrorsTemp);
	}
	return;
}


void sprintAttributeSQL(char *target, char *name, int value)
{
	if(strcmp(name,"EngineBase")==0){sprintf(target,"SELECT liter,cc,cid,cylinders,blocktype  from enginebase WHERE enginebaseid = %d;",value);return;}
	if(strcmp(name,"SubModel")==0){sprintf(target,"SELECT submodelname from submodel WHERE submodelid = %d;",value);return;}
	if(strcmp(name,"DriveType")==0){sprintf(target,"SELECT drivetypename from drivetype WHERE drivetypeid = %d;",value);return;}
	if(strcmp(name,"Aspiration")==0){sprintf(target,"SELECT aspirationname from aspiration WHERE aspirationid = %d;",value);return;}
	if(strcmp(name,"FuelType")==0){sprintf(target,"SELECT fueltypename from fueltype WHERE fueltypeid = %d;",value);return;}
	if(strcmp(name,"FrontBrakeType")==0){sprintf(target,"SELECT braketypename from braketype WHERE braketypeid = %d;",value);return;}
	if(strcmp(name,"RearBrakeType")==0){sprintf(target,"SELECT braketypename from braketype WHERE braketypeid = %d;",value);return;}
	if(strcmp(name,"BrakeABS")==0){sprintf(target,"SELECT brakeabsname from brakeabs WHERE brakeabsid = %d;",value);return;}
	if(strcmp(name,"MfrBodyCode")==0){sprintf(target,"SELECT mfrbodycodename from mfrbodycode WHERE mfrbodycodeid = %d;",value);return;}
	if(strcmp(name,"BodyNumDoors")==0){sprintf(target,"SELECT bodynumdoors from bodynumdoors WHERE bodynumdoorsid = %d;",value);return;}
	if(strcmp(name,"BodyType")==0){sprintf(target,"SELECT bodytypename from bodytype WHERE bodytypeid = %d;",value);return;}
	if(strcmp(name,"EngineDesignation")==0){sprintf(target,"SELECT enginedesignationname from enginedesignation WHERE enginedesignationid = %d;",value);return;}
	if(strcmp(name,"EngineVIN")==0){sprintf(target,"SELECT enginevinname from enginevin WHERE enginevinid = %d;",value);return;}
	if(strcmp(name,"EngineVersion")==0){sprintf(target,"SELECT engineversion from engineversion WHERE engineversionid = %d;",value);return;}
	if(strcmp(name,"EngineMfr")==0){sprintf(target,"SELECT mfrname from mfr WHERE mfrid = %d;",value);return;}
	if(strcmp(name,"FuelDeliveryType")==0){sprintf(target,"SELECT fueldeliverytypename from fueldeliverytype WHERE fueldeliverytypeid = %d;",value);return;}
	if(strcmp(name,"FuelDeliverySubType")==0){sprintf(target,"SELECT fueldeliverysubtypename from fueldeliverysubtype WHERE fueldeliverysubtypeid = %d;",value);return;}
	if(strcmp(name,"FuelSystemControlType")==0){sprintf(target,"SELECT fuelsystemcontroltypename from fuelsystemcontroltype WHERE fuelsystemcontroltypeid = %d;",value);return;}
	if(strcmp(name,"FuelSystemDesign")==0){sprintf(target,"SELECT fuelsystemdesignname from fuelsystemdesign WHERE fuelsystemdesignid = %d;",value);return;}
	if(strcmp(name,"CylinderHeadType")==0){sprintf(target,"SELECT cylinderheadtypename from cylinderheadtype WHERE cylinderheadtypeid = %d;",value);return;}
	if(strcmp(name,"IgnitionSystemType")==0){sprintf(target,"SELECT ignitionsystemtypename from ignitionsystemtype WHERE ignitionsystemtypeid = %d;",value);return;}
	if(strcmp(name,"TransmissionMfrCode")==0){sprintf(target,"SELECT transmissionmfrcode from transmissionmfrcode WHERE transmissionmfrcodeid = %d;",value);return;}
	if(strcmp(name,"TransmissionBase")==0){sprintf(target,"SELECT transmissioncontroltypename,transmissiontypename,transmissionnumspeeds from transmissionbase,transmissiontype, transmissionnumspeeds,transmissioncontroltype WHERE transmissionbase.transmissiontypeid=transmissiontype.transmissiontypeid AND transmissionbase.transmissionnumspeedsid=transmissionnumspeeds.transmissionnumspeedsid AND transmissionbase.transmissioncontroltypeid=transmissioncontroltype.transmissioncontroltypeid AND transmissionbase.transmissionbaseid = %d;",value);return;}
	if(strcmp(name,"TransmissionType")==0){sprintf(target,"SELECT transmissiontypename from transmissiontype WHERE transmissiontypeid = %d;",value);return;}
	if(strcmp(name,"TransmissionControlType")==0){sprintf(target,"SELECT transmissioncontroltypename from transmissioncontroltype WHERE transmissioncontroltypeid = %d;",value);return;}
	if(strcmp(name,"TransmissionNumSpeeds")==0){sprintf(target,"SELECT transmissionnumspeeds from transmissionnumspeeds WHERE transmissionnumspeedsid = %d;",value);return;}
	if(strcmp(name,"TransmissionMfr")==0){sprintf(target,"SELECT mfrname from mfr WHERE mfrid = %d;",value);return;}
	if(strcmp(name,"BedLength")==0){sprintf(target,"SELECT bedlength from bedlength WHERE bedlengthid = %d;",value);return;}
	if(strcmp(name,"BedType")==0){sprintf(target,"SELECT bedtypename from bedtype WHERE bedtypeid = %d;",value);return;}
	if(strcmp(name,"WheelBase")==0){sprintf(target,"SELECT wheelbase from wheelbase WHERE wheelbaseid = %d;",value);return;}
	if(strcmp(name,"BrakeSystem")==0){sprintf(target,"SELECT brakesystemname from brakesystem WHERE brakesystemid = %d;",value);return;}
	if(strcmp(name,"Region")==0){sprintf(target,"SELECT regionname from region WHERE regionid = %d;",value);return;}
	if(strcmp(name,"FrontSpringType")==0){sprintf(target,"SELECT springtypename from springtype WHERE springtypeid = %d;",value);return;}
	if(strcmp(name,"RearSpringType")==0){sprintf(target,"SELECT springtypename from springtype WHERE springtypeid = %d;",value);return;}
	if(strcmp(name,"SteeringSystem")==0){sprintf(target,"SELECT steeringsystemname from steeringsystem WHERE steeringsystemid = %d;",value);return;}
	if(strcmp(name,"SteeringType")==0){sprintf(target,"SELECT steeringtypename from steeringtype WHERE steeringtypeid = %d;",value);return;}
	if(strcmp(name,"ValvesPerEngine")==0){sprintf(target,"SELECT valvesperengine from valves WHERE valvesid = %d;",value);return;}
	target[0]=0;//Unknown attribute name
	return;
}

void sprintAttributeWhere(char *target, char *name, int value)
{
	if(strcmp(name,"EngineBase")==0){sprintf(target,"enginebase.enginebaseid = %d and ",value);return;}
	if(strcmp(name,"SubModel")==0){sprintf(target,"submodelid = %d and ",value);return;}
	if(strcmp(name,"DriveType")==0){sprintf(target,"drivetypeid = %d and ",value);return;}
	if(strcmp(name,"Aspiration")==0){sprintf(target,"aspirationid = %d and ",value);return;}
	if(strcmp(name,"FuelType")==0){sprintf(target,"fueltypeid = %d and ",value);return;}
	if(strcmp(name,"FrontBrakeType")==0){sprintf(target,"frontbraketypeid = %d and ",value);return;}
	if(strcmp(name,"RearBrakeType")==0){sprintf(target,"rearbraketypeid = %d and ",value);return;}
	if(strcmp(name,"BrakeABS")==0){sprintf(target,"brakeabsid = %d and ",value);return;}
	if(strcmp(name,"MfrBodyCode")==0){sprintf(target,"mfrbodycodeid = %d and ",value);return;}
	if(strcmp(name,"BodyNumDoors")==0){sprintf(target,"bodynumdoorsid = %d and ",value);return;}
	if(strcmp(name,"BodyType")==0){sprintf(target,"bodytypeid = %d and ",value);return;}
	if(strcmp(name,"EngineDesignation")==0){sprintf(target,"enginedesignationid = %d and ",value);return;}
	if(strcmp(name,"EngineVIN")==0){sprintf(target,"enginevinid = %d and ",value);return;}
	if(strcmp(name,"EngineVersion")==0){sprintf(target,"engineversionid = %d and ",value);return;}
	if(strcmp(name,"EngineMfr")==0){sprintf(target,"mfrid = %d and ",value);return;}
	if(strcmp(name,"FuelDeliveryType")==0){sprintf(target,"fueldeliverytypeid = %d and ",value);return;}
	if(strcmp(name,"FuelDeliverySubType")==0){sprintf(target,"fueldeliverysubtypeid = %d and ",value);return;}
	if(strcmp(name,"FuelSystemControlType")==0){sprintf(target,"fuelsystemcontroltypeid = %d and ",value);return;}
	if(strcmp(name,"FuelSystemDesign")==0){sprintf(target,"fuelsystemdesignid = %d and ",value);return;}
	if(strcmp(name,"CylinderHeadType")==0){sprintf(target,"cylinderheadtypeid = %d and ",value);return;}
	if(strcmp(name,"IgnitionSystemType")==0){sprintf(target,"ignitionsystemtypeid = %d and ",value);return;}
	if(strcmp(name,"TransmissionMfrCode")==0){sprintf(target,"transmissionmfrcodeid = %d and ",value);return;}
	if(strcmp(name,"TransmissionBase")==0){sprintf(target,"transmissionbase.transmissionbaseid = %d and ",value);return;}
	if(strcmp(name,"TransmissionType")==0){sprintf(target,"transmissiontypeid = %d and ",value);return;}
	if(strcmp(name,"TransmissionControlType")==0){sprintf(target,"transmissioncontroltypeid = %d and ",value);return;}
	if(strcmp(name,"TransmissionNumSpeeds")==0){sprintf(target,"transmissionnumspeedsid = %d and ",value);return;}
	if(strcmp(name,"TransmissionMfr")==0){sprintf(target,"mfrid = %d and ",value);return;}
	if(strcmp(name,"BedLength")==0){sprintf(target,"bedlengthid = %d and ",value);return;}
	if(strcmp(name,"BedType")==0){sprintf(target,"bedtypeid = %d and ",value);return;}
	if(strcmp(name,"WheelBase")==0){sprintf(target,"wheelbaseid = %d and ",value);return;}
	if(strcmp(name,"BrakeSystem")==0){sprintf(target,"brakesystemid = %d and ",value);return;}
	if(strcmp(name,"Region")==0){sprintf(target,"regionid = %d and ",value);return;}
	if(strcmp(name,"FrontSpringType")==0){sprintf(target,"frontspringtypeid = %d and ",value);return;}
	if(strcmp(name,"RearSpringType")==0){sprintf(target,"rearspringtypeid = %d and ",value);return;}
	if(strcmp(name,"SteeringSystem")==0){sprintf(target,"steeringsystemid = %d and ",value);return;}
	if(strcmp(name,"SteeringType")==0){sprintf(target,"steeringtypeid = %d and ",value);return;}
	if(strcmp(name,"ValvesPerEngine")==0){sprintf(target,"valvesid = %d and ",value);return;}
	target[0]=0;//Unknown attribute name
	return;
}




// determine which system group an attribute is in. this is for determining what tables to join in a validation query for the sake of effeciency

int systemGroupOfAttribute(char *attr)
{
	if(strcmp(attr,"Region")==0){return 0;}
	if(strcmp(attr,"SubModel")==0){return 0;}

	if(strcmp(attr,"DriveType")==0){return 1;}

	if(strcmp(attr,"BrakeABS")==0){return 2;}
	if(strcmp(attr,"BrakeSystem")==0){return 2;}
	if(strcmp(attr,"FrontBrakeType")==0){return 2;}
	if(strcmp(attr,"RearBrakeType")==0){return 2;}

	if(strcmp(attr,"EngineBase")==0){return 3;}
	if(strcmp(attr,"EngineVIN")==0){return 3;}
	if(strcmp(attr,"EngineVersion")==0){return 3;}
	if(strcmp(attr,"EngineMfr")==0){return 3;}
	if(strcmp(attr,"EngineDesignation")==0){return 3;}
	if(strcmp(attr,"FuelDeliverySubType")==0){return 3;}
	if(strcmp(attr,"FuelDeliveryType")==0){return 3;}
	if(strcmp(attr,"FuelSystemControlType")==0){return 3;}
	if(strcmp(attr,"FuelSystemDesign")==0){return 3;}
	if(strcmp(attr,"Aspiration")==0){return 3;}
	if(strcmp(attr,"IgnitionSystemType")==0){return 3;}
	if(strcmp(attr,"ValvesPerEngine")==0){return 3;}
	if(strcmp(attr,"CylinderHeadType")==0){return 3;}
	if(strcmp(attr,"FuelType")==0){return 3;}
	if(strcmp(attr,"PowerOutput")==0){return 3;}

	if(strcmp(attr,"BodyNumDoors")==0){return 4;}
	if(strcmp(attr,"BodyType")==0){return 4;}

	if(strcmp(attr,"MfrBodyCode")==0){return 5;}

	if(strcmp(attr,"TransElecControlled")==0){return 6;}
	if(strcmp(attr,"TransmissionBase")==0){return 6;}
	if(strcmp(attr,"TransmissionControlType")==0){return 6;}
	if(strcmp(attr,"TransmissionMfr")==0){return 6;}
	if(strcmp(attr,"TransmissionMfrCode")==0){return 6;}
	if(strcmp(attr,"TransmissionNumSpeeds")==0){return 6;}
	if(strcmp(attr,"TransmissionType")==0){return 6;}

	if(strcmp(attr,"WheelBase")==0){return 7;}

	if(strcmp(attr,"SteeringSystem")==0){return 8;}
	if(strcmp(attr,"SteeringType")==0){return 8;}

	if(strcmp(attr,"BedLength")==0){return 9;}
	if(strcmp(attr,"BedType")==0){return 9;}

	if(strcmp(attr,"FrontSpringType")==0){return 10;}
	if(strcmp(attr,"RearSpringType")==0){return 10;}

	if(strcmp(attr,"VehicleType")==0){return 11;}


	return 12;
}
#endif



// compare function for qsort
// basevid/parttype/position/part/mfrlabel/qualifiers
int appSortCompare(const void *a, const void *b)
{
	struct ACESapp *ptra =  *(struct ACESapp**)a;
	struct ACESapp *ptrb = *(struct ACESapp**)b;
	int strCmpResults;
	char tempStr[1024]="";
	char attributeStrA[1024]="";
	char attributeStrB[1024]="";
	int i; 

    if(ptra->basevid > ptrb->basevid)
    {//A basevid > B basevid
     return(+1);
    }
    else
    {//A basevid <= B basevid
     if (ptra->basevid == ptrb->basevid)
     {//A basevid = B basevid -  now compare secondary stuff

      if(ptra->parttype > ptrb->parttype)
      {// A parttype > B parttype
       return(+1);
      }
      else
      {// A parttype <= B parttype
       if(ptra->parttype == ptrb->parttype)
       {// basebids are equal, parttypes are equal

        if(ptra->position > ptrb->position)
        {//A pos > B pos
         return(+1);
        }
        else
        {//A pos <= B pos
         if(ptra->position == ptrb->position)
         {// basebids are equal, parttypes are equal, positions are equal
          strCmpResults=strcmp(ptra->part, ptrb->part);
          if(strCmpResults>0)
          {// ptra->part > ptrb->part
           return(+1);
          }
          else
          {//ptra->part <= ptrb->part
           if(strCmpResults==0)
           {/// basebids are equal, parttypes are equal, positions are equal, parts are equal
            strCmpResults=strcmp(ptra->mfrlabel, ptrb->mfrlabel);
            if(strCmpResults>0)
            {// mfrlabels A > B
             return(+1);
            }
            else
            {// mfrlabels A <= B
             if(strCmpResults==0)
             {
              // now contemplate attribute pairs and notes - this requires inflating them into strings (like: Submodel:103;EngineBase:33;Green Paint)
              attributeStrA[0]=0; attributeStrB[0]=0;
              for(i=0; i<=ptra->attributeCount-1; i++){snprintf(tempStr,1024,"%s:%d;",ptra->attributeNames[i],ptra->attributeValues[i]); strcat(attributeStrA,tempStr);} strcat(attributeStrA,ptra->notes);
              for(i=0; i<=ptrb->attributeCount-1; i++){snprintf(tempStr,1024,"%s:%d;",ptrb->attributeNames[i],ptrb->attributeValues[i]); strcat(attributeStrB,tempStr);} strcat(attributeStrB,ptrb->notes);
              strCmpResults=strcmp(attributeStrA,attributeStrB);

              if(strCmpResults>0)
              {// qualifiers A > qualifiers B
               return(+1);
              }
              else
              {//qualifiers A <= qualifiers B
               if(strCmpResults==0)
               {// basebids are equal, parttypes are equal, positions are equal, parts are equal, mfrlabels are equal, qualifiers are equal,
                return(0);
               }
               else
               {//qualifiers A < qualifiers B
                return(-1);
               }
              }
             }
             else
             {// mfrlabel A < mfrlabel B
              return(-1);
             }
            }
           }
           else
           {// A part < B part
            return(-1);
           }
          }
         }
         else
         {//A pos < B pos
          return(-1);
         }
        }
       }
       else
       {//  A parttype < B parttype
        return(-1);
       }
      }
     }
     else
     {//A basevid < B basevid
      return(-1);
     }
    }
}
