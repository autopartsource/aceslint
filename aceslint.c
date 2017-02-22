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

 MYSQL dbPCDB;
 MYSQL_ROW dbPCDBRow;
 MYSQL_RES *dbPCDBRecset;

 char sql_command[4096]= "";

#endif


int vcdb_used;
int pcdb_used;



struct ACESapp
{
        char action;
        int id;
        int basevid;
        int partsListid;
        char *notes;
        int position;
        int parttype;
        int qty;
	int mfrlabelid;
	int assetid;
	int assetorder;
	int attributeCount;
	char attributeTokens[8];
	int attributeValues[8];
};

struct MMY
{
	char makeName[100];
	char modelName[100];
	int year;
};




char *sprintNicePartTypeName(char *target,int parttypeid);
char *sprintNicePositionName(char *target,int positionid);
char *sprintNiceQualifiers(char *target, char *errors, struct ACESapp *app);
void sprintAttributeSQL(char *target, char *name, int value);
void sprintAttributeWhere(char *target, char *name, int value);
void sprintSQLclausesForApp(char *fromClause, char *whereClause, struct ACESapp *app);
int systemGroupOfAttribute(char *attr);
int appSortCompare(const void *a, const void *b);
char vcdbAtrributeToken(char *ref);
char *sprintVCdbAtrributeFromToken(char *target,char token);
char *sprintNiceMMY(char *target,int basevid, char delimiter);
void fillNiceMMYstruct(struct MMY *targetMMY,int basevid);
void sprintNiceAttribute(char *target, char *errors, struct ACESapp *app, int attributeIndex);

int main(int arg_count, char *args[])
{
	char aceslint_version[8]="0.1.0";
	int i,j;
	char strTemp[4096];
	char *charTempPtrA;
	char *charTempPtrB;
	char input_file_name[256]="";
	char output_xml_filename[256]="";
	char assessment_filename[256]="";
	char part_translation_filename[256]="";
	FILE *part_translation_fileptr=NULL;
	FILE *output_xml_fileptr=NULL;
	FILE *assessment_fileptr=NULL;
	char vcdb_name[256] = "";
	char pcdb_name[256] = "";
	char database_host[256] = "localhost";
	char database_user[64] = "";
	char database_pass[64]= "";
	int verbosity=1;
	int printed_header=0;
	int extract_parts=0;
	int extract_parttypes=0;
	int extract_mfrlabels=0;
	int extract_assets=0;
	int extract_partassetlinks=0;
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


	int parttypeList[1024];
	int parttypeListCount=0;
	int database_support=0;

	vcdb_used=0;
	pcdb_used=0;

#ifdef WITH_MYSQL
	database_support=1;
#endif


        if(arg_count == 1 || strcmp(args[arg_count-1],"?")==0)
        {// print usage 

		if(database_support)
		{
			printf("version:%s  compiled with mysql database support\n",aceslint_version);
			printf("\nbasic usage: aceslint inputfilename [-v <verbosity 0-9>] [-pcdb <pcdb database name> -vcdb <vcdb database name> [-dbh <database host> -dbu <datebase user> -dbp <datebase password>]]\n\n");
		}
		else
		{
			printf("\nversion:%s  NOT compiled with database support\n",aceslint_version);
			printf("\nbasic usage: aceslint inputfilename\n\n");
		}

		printf("options:\n");
		printf("\t-pcdb <PCdb database name> (example: \"-pcdb pcdb20170210\")\n");
		printf("\t-vcdb <VCdb database name> (example: \"-vcdb vcdb20170127\")\n");
		printf("\t-dbh <database host> (optional - \"localhost\" is assumed)\n");
		printf("\t-dbu <database user> (optional - \"\" is assumed)\n");
		printf("\t-dbp <database password> (optional - \"\" is assumed)\n");
		printf("\t-v <verbosity level 0-9> (optional - 1 is assumed)\n");
		printf("\t-ignorenaparts (ignore apps with \"NA\" as the part number)\n");
		printf("\t-parttranslationfile <filename> (translate part number by a 2-column, tab-delimited interchange. Apps with no interchange will be dropped)\n");
		printf("\t-filterbyyears <start modelyear> <end modelyear> (discard all apps outside given range. ex: \"--filterbyyears 2010 2012\" only preserves 2010,2011, 2012)\n");
		printf("\t-includeparttypeids <parttypeid1,parttypeid2,parttypeid3,,,> (discard all apps outside of given parttypeids. ex \"-includeparttypeids 6832\" only preserves Cabin Air Filters)\n");
		printf("\t-excludeparttypeids <parttypeid1,parttypeid2,parttypeid3,,,> (discard all apps in given parttypeids. ex: \"-excludeparttypeids 6832\" discards Cabin Air Filters)\n");
		printf("\t-includemakeids <makeid1,makeid2,makeid3,,,> (discard all apps outside of given makeID's. ex \"-includemakeids 75,76\" only preserves Lexus and Toyota apps)\n");
		printf("\t-excludemakeids <makeid1,makeid2,makeid3,,,> (discard all apps in given makeID's. ex \"-excludemakeids 75,76\" discards Lexus and Toyota apps)\n");
		printf("\t-extractparts (surpress all other output and dump distinct list of part numbers found in the input file)\n");
		printf("\t-extractparttypes (surpress all other output and dump distinct list of part types found in the input file. If pcdb is provided, human-readable names are included.)\n");
		printf("\t-extractmfrlabels (surpress all other output and dump distinct list of \"MfrLabels\" found in input file.)\n");
		printf("\t-extractassets (surpress all other output and dump distinct list of assets names found in the input file)\n");
		printf("\t-flattenmethod <method number> (export a \"flat\" list of applications as tab-delimited data. Method 1 is VCdb-coded values, Method 2 is human-readable)\n");
		printf("\t-assessment <filename> (export an e-cat style assessment spreadsheet in Excel 2003 XML format)\n\n");

		exit(1);
        }

	strcpy(input_file_name,args[1]);

	for(i=1;i<=arg_count-1;i++)
	{
		if(strcmp(args[i],"-vcdb")==0 && i<(arg_count-1)){if(database_support){strcpy(vcdb_name,args[i + 1]); vcdb_used=1;}else{printf("-vcdb option requires that aceslint was compiled with database support\n"); exit(1);}}
		if(strcmp(args[i],"-pcdb")==0 && i<(arg_count-1)){if(database_support){strcpy(pcdb_name,args[i + 1]); pcdb_used=1;}else{printf("-pcdb option requires that aceslint was compiled with database support\n"); exit(1);}}
		if(strcmp(args[i],"-dbh")==0 && i<(arg_count-1)){strcpy(database_host,args[i + 1]);}
		if(strcmp(args[i],"-dbu")==0 && i<(arg_count-1)){strcpy(database_user,args[i + 1]);}
		if(strcmp(args[i],"-dbp")==0 && i<(arg_count-1)){strcpy(database_pass,args[i + 1]);}
		if(strcmp(args[i],"-v")==0 && i<(arg_count-1)){verbosity=atoi(args[i + 1]);}
		if(strcmp(args[i],"-ignorenaparts")==0){ignore_na=1;}
		if(strcmp(args[i],"-filterbyyears")==0 && i<(arg_count-2)){filterfromyear=atoi(args[i + 1]); filtertoyear=atoi(args[i + 2]);}

		if((strcmp(args[i],"-includemakeids")==0 || strcmp(args[i],"-excludemakeids")==0) && i<(arg_count-1))
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
			if(strcmp(args[i],"-includemakeids")==0){makename_filter_mode=1;}
			if(strcmp(args[i],"-excludemakeids")==0){makename_filter_mode=2;}
		}

		if((strcmp(args[i],"-includeparttypeids")==0 || strcmp(args[i],"-excludeparttypeids")==0) && i<(arg_count-1))
		{
			if(strlen(args[i + 1])<1023)
			{
				strcpy(strTemp,args[i + 1]);
				charTempPtrA=strTemp; charTempPtrB=strTemp;
				while (charTempPtrA != NULL)
				{
					strsep(&charTempPtrB, ",");
					if(strlen(charTempPtrA)>6){printf("parttype id (%s) was too long - The limit is 6 characters\n",charTempPtrA); exit(1);}
					parttypeids_list[parttypeids_list_count]=atoi(charTempPtrA);
					if(parttypeids_list[parttypeids_list_count]<1 || parttypeids_list[parttypeids_list_count]>999999){printf("parttype id (%s) was out of range - 0-99999\n",charTempPtrA); exit(1);}
					parttypeids_list_count++;
					charTempPtrA = charTempPtrB;
				}
			}
			else{printf("parttypes list was too long\n"); exit(1);}
			if(strcmp(args[i],"-includeparttypeids")==0){parttypeids_filter_mode=1;}
			if(strcmp(args[i],"-excludeparttypeids")==0){parttypeids_filter_mode=2;}
		}

		if(strcmp(args[i],"-extractparts")==0){verbosity=0; extract_parts=1;}
		if(strcmp(args[i],"-extractparttypes")==0){verbosity=0; extract_parttypes=1;}
		if(strcmp(args[i],"-extractmfrlabels")==0){verbosity=0; extract_mfrlabels=1;}
		if(strcmp(args[i],"-extractassets")==0){verbosity=0; extract_assets=1;}
		if(strcmp(args[i],"-extractpartassetlinks")==0){verbosity=0; extract_partassetlinks=1;}
		if(strcmp(args[i],"-flattenmethod")==0 && i<(arg_count-1)){verbosity=0; flatten=atoi(args[i + 1]);}
		if(strcmp(args[i],"-parttranslationfile")==0 && i<(arg_count-1)){strcpy(part_translation_filename,args[i + 1]);}
		if(strcmp(args[i],"-outputxmlfile")==0 && i<(arg_count-1)){strcpy(output_xml_filename,args[i + 1]);}
		if(strcmp(args[i],"-assessment")==0 && i<(arg_count-1)){strcpy(assessment_filename,args[i + 1]);}
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
	tempApp.notes=(char *) malloc(sizeof(char)*1024);

	char *partsList[20000];
	int partsListCount=0;
	char tempPart[256];

	char *mfrlabelList[10000];
	int mfrlabelListCount=0;
	char tempmfrlabel[256];

	char *assetList[20000];
	int assetListCount=0;
	int assetsFoundInXML=0;
	char tempAssetName[256];

	char document_title[256]="";
	char vcdb_version[32]="";
	char pcdb_version[32]="";
	char qdb_version[32]="";

	char attributeStrA[4096];
	char attributeStrB[4096];
	int maxNoteSize=0;
	char attributeTokenTemp;
	char attributeNameTemp[256];

	int invalid_count=0;
	int found;

	int part_asset_connections[2][100000];
	int part_asset_connections_count=0;

	char partTranslations[20000][2][32];
	int partTranslationsCount=0;
	int field_index=0;
	int input_byte;


	char strPartTypeName[256]="";
	char strPositionName[256]="";

	char niceMMYtemp[512];

	struct MMY structMMYtemp;

	char analysisFileStr[10000];
	char referenced_VCdb_verson[32]="";
	char referenced_PCdb_verson[32]="";
	char excelTabColorXMLtag[50];


	if(part_translation_filename[0])
	{// translation filename was provied
		part_translation_fileptr = fopen(part_translation_filename,"r"); if(part_translation_fileptr == (FILE *)0){ printf("Error opening part translation file\n"); exit(1);}
		field_index = 0; strTemp[0]=0; j=0;
		while((input_byte = fgetc(part_translation_fileptr)) != EOF)
		{
			if(input_byte == 13){continue;}			// ignore CR's
			if(input_byte == 9)
			{// this byte was a field delimiter - advance field index and grab next byte
				if(field_index<2 && j<31)
				{// only copy the field if we are in the first two columns and the accumulated string is not too long
					strcpy(partTranslations[partTranslationsCount][field_index],strTemp);
				}
				strTemp[0]=0; j=0; field_index++; continue;
			}
			if(input_byte == 10)
		        {// just read in a whole row
				if(field_index<2 && strlen(strTemp)<32)
				{// only copy the field if we are in the first two columns and the accumulated string is not too long
					strcpy(partTranslations[partTranslationsCount][field_index],strTemp);
					partTranslationsCount++;
				}
				strTemp[0]=0; j=0; field_index = 0; continue;
			}
			if(j>30){printf("Part translation file contains a field that is too long (on line# %d)\n",partTranslationsCount+1); exit(1);}
			strTemp[j] = input_byte;
			strTemp[j+1] = 0; // tack on a null-terminator after the current byte
			j++;
		}
		if(field_index==1 && j<31)
		{// there was no LF at the end of last line - consume what's still in the temp str
			strTemp[j+1] = 0; // tack on a null-terminator after the current byte
			partTranslationsCount++;
		}
		fclose(part_translation_fileptr);
	}
	if(verbosity>0 && partTranslationsCount){printf("%d parts in translations table\n",partTranslationsCount);}

	if(output_xml_filename[0]){output_xml_fileptr = fopen(output_xml_filename,"w"); if(output_xml_fileptr == (FILE *)0){ printf("Error opening output xml file\n"); exit(1);}}


	//initialize mysql client - for VCDB connection
#ifdef WITH_MYSQL
	if(vcdb_used)
	{
		if(!mysql_init(&dbVCDB)){printf("mysql client (VCDB) didn't initialize\n"); exit(1);}
		if(!mysql_real_connect(&dbVCDB,database_host,database_user,database_pass,vcdb_name,0,NULL,0)){printf("database connection (VCdb) not established\n"); exit(1);}
		sprintf(sql_command,"select versiondate from version;"); if(mysql_query(&dbVCDB,sql_command)){printf("\nSQL Error\n%s\n",sql_command);exit(1);} dbVCDBRecset = mysql_store_result(&dbVCDB);
		if((dbVCDBRow = mysql_fetch_row(dbVCDBRecset))){strcpy(referenced_VCdb_verson,dbVCDBRow[0]);}mysql_free_result(dbVCDBRecset);
	}
	if(pcdb_used)
	{
		if(!mysql_init(&dbPCDB)){printf("mysql client (PCDB) didn't initialize\n"); exit(1);}
		if(!mysql_real_connect(&dbPCDB,database_host,database_user,database_pass,pcdb_name,0,NULL,0)){printf("database connection (PCdb) not established\n"); exit(1);}
		sprintf(sql_command,"select versiondate from version;"); if(mysql_query(&dbPCDB,sql_command)){printf("\nSQL Error\n%s\n",sql_command);exit(1);} dbPCDBRecset = mysql_store_result(&dbPCDB);
		if((dbPCDBRow = mysql_fetch_row(dbPCDBRecset))){strcpy(referenced_PCdb_verson,dbPCDBRow[0]);}mysql_free_result(dbPCDBRecset);
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

	strcpy(xmlPathString,"/ACES/App");
	xmlContext = xmlXPathNewContext(doc);
	xmlResult = xmlXPathEvalExpression(BAD_CAST xmlPathString, xmlContext);
	xmlXPathFreeContext(xmlContext);
 	nodesetApps = xmlResult->nodesetval;

	for (i=0; i < nodesetApps->nodeNr; i++)
	{
		include_app=1;

		// extract xml tag data from thei app node into temp app structure - we may decide to drop this app later - no need to malloc space for it yet.
		tempApp.action=' ';
		tempApp.id=-1;
		tempApp.basevid=-1;
		tempApp.partsListid=-1;
		tempApp.mfrlabelid=-1;
		tempApp.assetid=-1;
		tempApp.assetorder=0;
		*tempApp.notes=0;
		tempApp.parttype=-1;
		tempApp.position=1;	//initialize to PCdb's "N/A" (position id 1)
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
			if ((!xmlStrcmp(cur->name, (const xmlChar *)"Part")))
			{
				xmlCharPtr=xmlNodeGetContent(cur); strcpy(tempPart,(char *)xmlCharPtr); xmlFree(xmlCharPtr);

				if(partTranslationsCount)
				{// there is a translation list in play
					found=0;
					for(j=0; j<=partTranslationsCount-1; j++){if(strcmp(tempPart,partTranslations[j][0])==0){found=1; break;}}
					if(found)
					{// found this app's part in the translation list - overwrite the tempPart sting that currently contains native (original) part
						strcpy(tempPart,partTranslations[j][1]);
					}
					else
					{// no translation was found for this app's part - short the appnodes loop 
						include_app=0;;
					}
				}
				//look for existing part in "partsList". use the found id or add a new part to the list and inc the "count" - then use the new part id for this app
				found=0;
				for(j=0;j<=partsListCount-1;j++)
				{
					if(strcmp(tempPart,partsList[j])==0){found=1; tempApp.partsListid=j; break;}
				}
				if(!found)
				{
					partsList[partsListCount] = (char *) malloc(sizeof(char)*strlen(tempPart)+1);
					strcpy(partsList[partsListCount],tempPart);
					tempApp.partsListid=partsListCount;
					partsListCount++;
				}
			}

			//------------    optional tags ---------------------
			if ((!xmlStrcmp(cur->name, (const xmlChar *)"MfrLabel")))
			{
				xmlCharPtr=xmlNodeGetContent(cur); strcpy(tempmfrlabel,(char *)xmlCharPtr); xmlFree(xmlCharPtr);
				found=0;
				for(j=0;j<=mfrlabelListCount-1;j++)
				{
					if(strcmp(tempmfrlabel,mfrlabelList[j])==0){found=1; tempApp.mfrlabelid=j; break;}
				}
				if(!found)
				{
					mfrlabelList[mfrlabelListCount] = (char *) malloc(sizeof(char)*strlen(tempmfrlabel)+1);
					strcpy(mfrlabelList[mfrlabelListCount],tempmfrlabel);
					tempApp.mfrlabelid=mfrlabelListCount;
					mfrlabelListCount++;
				}
			}

			if((!xmlStrcmp(cur->name, (const xmlChar *)"AssetName")))
			{
				xmlCharPtr=xmlNodeGetContent(cur); strcpy(tempAssetName,(char *)xmlCharPtr);
				found=0;
				for(j=0;j<=assetListCount-1;j++)
				{
					if(strcmp(tempAssetName,assetList[j])==0){found=1; tempApp.assetid=j; break;}
				}
				if(!found)
				{
					assetList[assetListCount] = (char *) malloc(sizeof(char)*strlen(tempAssetName)+1);
					strcpy(assetList[assetListCount],tempAssetName);
					tempApp.assetid=assetListCount;
					assetListCount++;
				}
				xmlFree(xmlCharPtr); assetsFoundInXML=1;
			}

			if ((!xmlStrcmp(cur->name, (const xmlChar *)"AssetItemOrder"))){xmlCharPtr=xmlNodeGetContent(cur); tempApp.assetorder=atoi((char *)xmlCharPtr); xmlFree(xmlCharPtr);}

			if(!xmlStrcmp(cur->name, (const xmlChar *)"Note"))
			{
				if(strlen((char *)xmlCharPtr)>maxNoteSize){maxNoteSize=strlen((char *)xmlCharPtr);}
				xmlCharPtr=xmlNodeGetContent(cur); strcat(tempApp.notes,(char *)xmlCharPtr); strcat(tempApp.notes,"; "); xmlFree(xmlCharPtr);
			}

			attributeTokenTemp=vcdbAtrributeToken((char *)cur->name);
			if(attributeTokenTemp>0 && attributeTokenTemp<=40)
			{// this tag name (inside an "App" node is one of the 40 known VCdb-coded attributes
				tempApp.attributeTokens[tempApp.attributeCount]=attributeTokenTemp;
				xmlCharPtr=xmlGetProp(cur,(const xmlChar *)"id"); tempApp.attributeValues[tempApp.attributeCount]=atoi((char *)xmlCharPtr); xmlFree(xmlCharPtr);
				tempApp.attributeCount++;
			}

			cur = cur->next;
		}


		if(ignore_na && strcmp(partsList[tempApp.partsListid],"NA")==0)
		{// this app's part number is "NA" and we have elected to ignore NA's. free the malloc'd memory for this app and don't advance the app count
			include_app=0;
		}

#ifdef WITH_MYSQL
		if(vcdb_used && (filterfromyear>0 || filtertoyear<9999))
		{// modelyear filtration is in play
			sprintf(sql_command,"select yearid from basevehicle where basevehicleid=%d;",tempApp.basevid); if(mysql_query(&dbVCDB,sql_command)){printf("\nSQL Error\n%s\n",sql_command);exit(1);} dbVCDBRecset = mysql_store_result(&dbVCDB);
			if(dbVCDBRow = mysql_fetch_row(dbVCDBRecset))
			{
				year=atoi(dbVCDBRow[0]);
				if(year<filterfromyear || year>filtertoyear){include_app=0;}
			}
			mysql_free_result(dbVCDBRecset);
		}

		if(vcdb_used && makename_filter_mode)
		{// test this app against make include/exclude list

			if(makename_filter_mode==1)
			{// list provided is an include list
				sprintf(sql_command,"select makeid from basevehicle where basevehicleid=%d;",tempApp.basevid); if(mysql_query(&dbVCDB,sql_command)){printf("\nSQL Error\n%s\n",sql_command);exit(1);} dbVCDBRecset = mysql_store_result(&dbVCDB);
				if(dbVCDBRow = mysql_fetch_row(dbVCDBRecset))
				{
					found=0;
					for(j=0; j<=makeids_list_count-1;j++)
					{
						if(makeids_list[j]==atoi(dbVCDBRow[0]))
						{
							found=1; break;
						}
					}
					if(!found){include_app=0;}
				}mysql_free_result(dbVCDBRecset);
			}

			if(makename_filter_mode==2)
			{// list provided is an exclude list 
				sprintf(sql_command,"select makeid from basevehicle where basevehicleid=%d;",tempApp.basevid); if(mysql_query(&dbVCDB,sql_command)){printf("\nSQL Error\n%s\n",sql_command);exit(1);} dbVCDBRecset = mysql_store_result(&dbVCDB);
				if(dbVCDBRow = mysql_fetch_row(dbVCDBRecset))
				{
					for(j=0; j<=makeids_list_count-1;j++){if(makeids_list[j]==atoi(dbVCDBRow[0])){include_app=0; break;}}
				}mysql_free_result(dbVCDBRecset);
			}
		}
#endif


		if(parttypeids_filter_mode)
		{// test this app against parttype include/exclude list

			if(parttypeids_filter_mode==1)
			{// list provided is an include list
				found=0;
				for(j=0; j<=parttypeids_list_count-1;j++)
				{
					if(parttypeids_list[j]==tempApp.parttype)
					{
						found=1; break;
					}
				}
				if(!found){include_app=0;}
			}

			if(parttypeids_filter_mode==2)
			{// list provided is an exclude list 
				for(j=0; j<=parttypeids_list_count-1;j++)
				{
					if(parttypeids_list[j]==tempApp.parttype)
					{
						include_app=0; break;
					}
				}
			}
		}

		if(include_app)
		{// we did not find a reason to exclude this app. allocate memory and add its pointer to the array
			apps[apps_count] = (struct ACESapp *) malloc(sizeof(struct ACESapp));
			apps[apps_count]->action=tempApp.action;
			apps[apps_count]->id=tempApp.id;
			apps[apps_count]->basevid=tempApp.basevid;
			apps[apps_count]->partsListid=tempApp.partsListid;
			apps[apps_count]->mfrlabelid=tempApp.mfrlabelid;
			apps[apps_count]->assetid=tempApp.assetid;
			apps[apps_count]->assetorder=tempApp.assetorder;
			apps[apps_count]->notes = (char *) malloc(sizeof(char)*strlen(tempApp.notes)+1);
			strcpy(apps[apps_count]->notes,tempApp.notes);
			apps[apps_count]->parttype=tempApp.parttype;
			apps[apps_count]->position=tempApp.position;
			apps[apps_count]->qty=tempApp.qty;
			apps[apps_count]->attributeCount=tempApp.attributeCount;
			for(j=0; j<=tempApp.attributeCount-1; j++){apps[apps_count]->attributeTokens[j]=tempApp.attributeTokens[j]; apps[apps_count]->attributeValues[j]=tempApp.attributeValues[j];}
			apps_count++;
		}
	}

	// free xml nodeset - all data that we care about is now in the apps[] array 
	xmlXPathFreeObject(xmlResult);


	if(verbosity>0){printf("Title:%s\n",document_title);}
	if(verbosity>0){printf("VcdbVersionDate:%s\n",vcdb_version);}
	if(verbosity>0 && referenced_VCdb_verson[0]){printf("VCdb referenced:%s\n",referenced_VCdb_verson);}
	if(verbosity>0){printf("PcdbVersionDate:%s\n",pcdb_version);}
	if(verbosity>0 && referenced_PCdb_verson[0]){printf("PCdb referenced:%s\n",referenced_PCdb_verson);}
	if(verbosity>0){printf("Application count:%d\n",apps_count);}
	if(verbosity>0){printf("Unique Part count:%d\n",partsListCount);}
	if(verbosity>0){printf("Unique MfrLabel count:%d\n",mfrlabelListCount);}

	for(i=0;i<=apps_count-1;i++)
	{
		found=0;
		for(j=0;j<=parttypeListCount-1;j++)
		{
			if(apps[i]->parttype==parttypeList[j]){found=1; break;}
		}
		if(!found){parttypeList[parttypeListCount]=apps[i]->parttype; parttypeListCount++;}
	}
	if(verbosity>0){printf("Unique Parttypes count:%d\n",parttypeListCount);}


	if(assessment_filename[0])
	{
		assessment_fileptr = fopen(assessment_filename,"w"); if(assessment_fileptr == (FILE *)0){ printf("Error opening assessment file\n"); exit(1);}
		fprintf(assessment_fileptr,"<?xml version=\"1.0\"?><?mso-application progid=\"Excel.Sheet\"?><Workbook xmlns=\"urn:schemas-microsoft-com:office:spreadsheet\" xmlns:o=\"urn:schemas-microsoft-com:office:office\" xmlns:x=\"urn:schemas-microsoft-com:office:excel\" xmlns:ss=\"urn:schemas-microsoft-com:office:spreadsheet\" xmlns:html=\"http://www.w3.org/TR/REC-html40\"><DocumentProperties xmlns=\"urn:schemas-microsoft-com:office:office\"><Author>ACESlint</Author><LastAuthor>ACESlint</LastAuthor><Created>2017-02-20T01:10:23Z</Created><LastSaved>2017-02-20T02:49:36Z</LastSaved><Version>14.00</Version></DocumentProperties><OfficeDocumentSettings xmlns=\"urn:schemas-microsoft-com:office:office\"><AllowPNG/></OfficeDocumentSettings><ExcelWorkbook xmlns=\"urn:schemas-microsoft-com:office:excel\"><WindowHeight>7500</WindowHeight><WindowWidth>15315</WindowWidth><WindowTopX>120</WindowTopX><WindowTopY>150</WindowTopY><TabRatio>785</TabRatio><ProtectStructure>False</ProtectStructure><ProtectWindows>False</ProtectWindows></ExcelWorkbook><Styles><Style ss:ID=\"Default\" ss:Name=\"Normal\"><Alignment ss:Vertical=\"Bottom\"/><Borders/><Font ss:FontName=\"Calibri\" x:Family=\"Swiss\" ss:Size=\"11\" ss:Color=\"#000000\"/><Interior/><NumberFormat/><Protection/></Style><Style ss:ID=\"s62\"><NumberFormat ss:Format=\"Short Date\"/></Style><Style ss:ID=\"s64\" ss:Name=\"Hyperlink\"><Font ss:FontName=\"Calibri\" x:Family=\"Swiss\" ss:Size=\"11\" ss:Color=\"#0000FF\" ss:Underline=\"Single\"/></Style><Style ss:ID=\"s65\"><Font ss:FontName=\"Calibri\" x:Family=\"Swiss\" ss:Size=\"11\" ss:Color=\"#000000\" ss:Bold=\"1\"/><Interior ss:Color=\"#D9D9D9\" ss:Pattern=\"Solid\"/></Style></Styles><Worksheet ss:Name=\"Stats\"><Table ss:ExpandedColumnCount=\"2\" x:FullColumns=\"1\" x:FullRows=\"1\" ss:DefaultRowHeight=\"15\"><Column ss:Width=\"116.25\"/><Column ss:Width=\"225\"/>");
		fprintf(assessment_fileptr,"<Row><Cell><Data ss:Type=\"String\">Input Filename</Data></Cell><Cell><Data ss:Type=\"String\">%s</Data></Cell></Row>",input_file_name);
		fprintf(assessment_fileptr,"<Row><Cell><Data ss:Type=\"String\">Title</Data></Cell><Cell><Data ss:Type=\"String\">%s</Data></Cell></Row>",document_title);
		fprintf(assessment_fileptr,"<Row><Cell><Data ss:Type=\"String\">VcdbVersionDate</Data></Cell><Cell ss:StyleID=\"s62\"><Data ss:Type=\"String\">%s</Data></Cell></Row>",vcdb_version);
		fprintf(assessment_fileptr,"<Row><Cell><Data ss:Type=\"String\">Application count</Data></Cell><Cell><Data ss:Type=\"Number\">%d</Data></Cell></Row><Row>",apps_count);
		fprintf(assessment_fileptr,"<Cell><Data ss:Type=\"String\">Unique Part count</Data></Cell><Cell><Data ss:Type=\"Number\">%d</Data></Cell></Row>",partsListCount);
		fprintf(assessment_fileptr,"<Row><Cell><Data ss:Type=\"String\">Unique MfrLabel count</Data></Cell><Cell><Data ss:Type=\"Number\">%d</Data></Cell></Row>",mfrlabelListCount);
		fprintf(assessment_fileptr,"<Row><Cell><Data ss:Type=\"String\">Unique Parttypes count</Data></Cell><Cell><Data ss:Type=\"Number\">%d</Data></Cell></Row>",parttypeListCount);
		fprintf(assessment_fileptr,"<Row><Cell><Data ss:Type=\"String\">Validator Tool</Data></Cell><Cell><Data ss:Type=\"String\">ACESlint</Data></Cell></Row>");
		fprintf(assessment_fileptr,"<Row><Cell><Data ss:Type=\"String\">Validator project home</Data></Cell><Cell ss:StyleID=\"s64\" ss:HRef=\"https://github.com/autopartsource/aceslint\"><Data ss:Type=\"String\">github.com/autopartsource/aceslint</Data></Cell></Row>");

		fprintf(assessment_fileptr,"</Table><WorksheetOptions xmlns=\"urn:schemas-microsoft-com:office:excel\"><PageSetup><Header x:Margin=\"0.3\"/><Footer x:Margin=\"0.3\"/><PageMargins x:Bottom=\"0.75\" x:Left=\"0.7\" x:Right=\"0.7\" x:Top=\"0.75\"/></PageSetup><Selected/><ProtectObjects>False</ProtectObjects><ProtectScenarios>False</ProtectScenarios></WorksheetOptions></Worksheet>");

		fprintf(assessment_fileptr,"<Worksheet ss:Name=\"Parts\"><Table ss:ExpandedColumnCount=\"1\" x:FullColumns=\"1\" x:FullRows=\"1\" ss:DefaultRowHeight=\"15\">");
		for(j=0;j<=partsListCount-1;j++){fprintf(assessment_fileptr,"<Row><Cell><Data ss:Type=\"String\">%s</Data></Cell></Row>",partsList[j]);}
		fprintf(assessment_fileptr,"</Table><WorksheetOptions xmlns=\"urn:schemas-microsoft-com:office:excel\"><PageSetup><Header x:Margin=\"0.3\"/><Footer x:Margin=\"0.3\"/><PageMargins x:Bottom=\"0.75\" x:Left=\"0.7\" x:Right=\"0.7\" x:Top=\"0.75\"/></PageSetup><ProtectObjects>False</ProtectObjects><ProtectScenarios>False</ProtectScenarios></WorksheetOptions></Worksheet>");


		fprintf(assessment_fileptr,"<Worksheet ss:Name=\"Part Types\"><Table ss:ExpandedColumnCount=\"2\" x:FullColumns=\"1\" x:FullRows=\"1\" ss:DefaultRowHeight=\"15\"><Column ss:Index=\"2\" ss:AutoFitWidth=\"0\" ss:Width=\"183.75\"/>");
		for(j=0;j<=parttypeListCount-1;j++)
		{
			if(pcdb_used)
			{
				fprintf(assessment_fileptr,"<Row><Cell><Data ss:Type=\"Number\">%d</Data></Cell><Cell><Data ss:Type=\"String\">%s</Data></Cell></Row>",parttypeList[j],sprintNicePartTypeName(strPartTypeName,parttypeList[j]));
			}
			else
			{
				fprintf(assessment_fileptr,"<Row><Cell><Data ss:Type=\"Number\">%d</Data></Cell><Cell><Data ss:Type=\"String\"></Data></Cell></Row>",parttypeList[j]);
			}
		}
		fprintf(assessment_fileptr,"</Table><WorksheetOptions xmlns=\"urn:schemas-microsoft-com:office:excel\"><PageSetup><Header x:Margin=\"0.3\"/><Footer x:Margin=\"0.3\"/><PageMargins x:Bottom=\"0.75\" x:Left=\"0.7\" x:Right=\"0.7\" x:Top=\"0.75\"/></PageSetup><ProtectObjects>False</ProtectObjects><ProtectScenarios>False</ProtectScenarios></WorksheetOptions></Worksheet>");

		fprintf(assessment_fileptr,"<Worksheet ss:Name=\"MfrLabels\"><Table ss:ExpandedColumnCount=\"1\" x:FullColumns=\"1\" x:FullRows=\"1\" ss:DefaultRowHeight=\"15\"><Column ss:AutoFitWidth=\"0\" ss:Width=\"151.5\"/>");
		for(j=0;j<=mfrlabelListCount-1;j++){fprintf(assessment_fileptr,"<Row><Cell><Data ss:Type=\"String\">%s</Data></Cell></Row>",mfrlabelList[j]);}
		fprintf(assessment_fileptr,"</Table><WorksheetOptions xmlns=\"urn:schemas-microsoft-com:office:excel\"><PageSetup><Header x:Margin=\"0.3\"/><Footer x:Margin=\"0.3\"/><PageMargins x:Bottom=\"0.75\" x:Left=\"0.7\" x:Right=\"0.7\" x:Top=\"0.75\"/></PageSetup><ProtectObjects>False</ProtectObjects><ProtectScenarios>False</ProtectScenarios></WorksheetOptions></Worksheet>");
	}

	qsort(apps, apps_count, sizeof(struct ACESapp *), appSortCompare);

#ifdef WITH_MYSQL
	if(vcdb_used)
	{
		//check validity of basevids 
		if(verbosity>2){printf("checking for valid basevids\n");}
		if(assessment_filename[0]){fprintf(assessment_fileptr,"<Worksheet ss:Name=\"Invalid Base Vids\"><Table ss:ExpandedColumnCount=\"7\" x:FullColumns=\"1\" x:FullRows=\"1\" ss:DefaultRowHeight=\"15\"><Column ss:AutoFitWidth=\"0\" ss:Width=\"45\"/><Column ss:Width=\"77.25\"/><Column ss:Index=\"4\" ss:AutoFitWidth=\"0\" ss:Width=\"96\"/><Column ss:AutoFitWidth=\"0\" ss:Width=\"73.5\"/><Column ss:AutoFitWidth=\"0\" ss:Width=\"253.5\"/><Column ss:AutoFitWidth=\"0\" ss:Width=\"371.25\"/><Row><Cell ss:StyleID=\"s65\"><Data ss:Type=\"String\">App Id</Data></Cell><Cell ss:StyleID=\"s65\"><Data ss:Type=\"String\">Invalid BaseVid</Data></Cell><Cell ss:StyleID=\"s65\"><Data ss:Type=\"String\">Part</Data></Cell><Cell ss:StyleID=\"s65\"><Data ss:Type=\"String\">Part Type</Data></Cell><Cell ss:StyleID=\"s65\"><Data ss:Type=\"String\">Position</Data></Cell><Cell ss:StyleID=\"s65\"><Data ss:Type=\"String\">Qualifiers</Data></Cell><Cell ss:StyleID=\"s65\"><Data ss:Type=\"String\">Notes</Data></Cell></Row>"); excelTabColorXMLtag[0]=0;}
		invalid_count=0; printed_header=0;
		for(i=0;i<=apps_count-1;i++)
		{
			attributeStrA[0]=0;for(j=0; j<=apps[i]->attributeCount-1; j++){snprintf(strTemp,1024,"%s:%d;",sprintVCdbAtrributeFromToken(attributeNameTemp,apps[i]->attributeTokens[j]),apps[i]->attributeValues[j]); strcat(attributeStrA,strTemp);} strcat(attributeStrA,apps[i]->notes);
			sprintf(sql_command,"select makeid from basevehicle where basevehicleid=%d;",apps[i]->basevid); if(mysql_query(&dbVCDB,sql_command)){printf("\nSQL Error\n%s\n",sql_command);exit(1);} dbVCDBRecset = mysql_store_result(&dbVCDB);
			dbVCDBRow = mysql_fetch_row(dbVCDBRecset);
			if(dbVCDBRow==NULL)
			{// basevehicleid was not found in vcdb
				if(!printed_header && verbosity>2){printf("App Id\tBase vid\tPart Type\tPosition\tPart\tQualifiers\n"); printed_header=1;}
				if(verbosity>2){printf("%d\t%d\t%d\t%d\t%s\t%s\n",apps[i]->id,apps[i]->basevid,apps[i]->parttype,apps[i]->position,partsList[apps[i]->partsListid],attributeStrA);}
				if(assessment_filename[0]){fprintf(assessment_fileptr,"<Row><Cell><Data ss:Type=\"Number\">%d</Data></Cell><Cell><Data ss:Type=\"Number\">%d</Data></Cell><Cell><Data ss:Type=\"String\">%s</Data></Cell><Cell><Data ss:Type=\"String\">%s</Data></Cell><Cell><Data ss:Type=\"String\">%s</Data></Cell><Cell><Data ss:Type=\"String\">%s</Data></Cell><Cell><Data ss:Type=\"String\">%s</Data></Cell></Row>",apps[i]->id,apps[i]->basevid,partsList[apps[i]->partsListid],sprintNicePartTypeName(strPartTypeName,apps[i]->parttype),sprintNicePositionName(strPositionName,apps[i]->position),attributeStrA,apps[i]->notes);}
				invalid_count++;
			}
			mysql_free_result(dbVCDBRecset);
		}
		if(invalid_count){strcpy(excelTabColorXMLtag,"<TabColorIndex>10</TabColorIndex>");}
		if(assessment_filename[0]){fprintf(assessment_fileptr,"</Table><WorksheetOptions xmlns=\"urn:schemas-microsoft-com:office:excel\"><PageSetup><Header x:Margin=\"0.3\"/><Footer x:Margin=\"0.3\"/><PageMargins x:Bottom=\"0.75\" x:Left=\"0.7\" x:Right=\"0.7\" x:Top=\"0.75\"/></PageSetup><Print><ValidPrinterInfo/><HorizontalResolution>600</HorizontalResolution><VerticalResolution>600</VerticalResolution></Print>%s<FreezePanes/><FrozenNoSplit/><SplitHorizontal>1</SplitHorizontal><TopRowBottomPane>1</TopRowBottomPane><ActivePane>2</ActivePane><Panes><Pane><Number>3</Number></Pane><Pane><Number>2</Number><ActiveRow>0</ActiveRow></Pane></Panes><ProtectObjects>False</ProtectObjects><ProtectScenarios>False</ProtectScenarios></WorksheetOptions></Worksheet>",excelTabColorXMLtag);}
		if(verbosity>0){printf("Invalid basevids:%d\n",invalid_count);}



		//check validity of coded attributes 
		if(verbosity>2){printf("checking for valid attribute ids\n");}
		if(assessment_filename[0]){fprintf(assessment_fileptr,"<Worksheet ss:Name=\"Invalid VCdb Codes\"><Table ss:ExpandedColumnCount=\"9\" x:FullColumns=\"1\" x:FullRows=\"1\" ss:DefaultRowHeight=\"15\"><Column ss:AutoFitWidth=\"0\" ss:Width=\"45\"/><Column ss:AutoFitWidth=\"0\" ss:Width=\"78.75\"/><Column ss:AutoFitWidth=\"0\" ss:Width=\"99.75\"/><Column ss:Width=\"31.5\"/><Column ss:AutoFitWidth=\"0\" ss:Width=\"60\"/><Column ss:AutoFitWidth=\"0\" ss:Width=\"112.5\"/><Column ss:Width=\"43.5\"/><Column ss:AutoFitWidth=\"0\" ss:Width=\"237\"/><Column ss:AutoFitWidth=\"0\" ss:Width=\"319.5\"/><Row><Cell ss:StyleID=\"s65\"><Data ss:Type=\"String\">App Id</Data></Cell><Cell ss:StyleID=\"s65\"><Data ss:Type=\"String\">Make</Data></Cell><Cell ss:StyleID=\"s65\"><Data ss:Type=\"String\">Model</Data></Cell><Cell ss:StyleID=\"s65\"><Data ss:Type=\"String\">Year</Data></Cell><Cell ss:StyleID=\"s65\"><Data ss:Type=\"String\">Part</Data></Cell><Cell ss:StyleID=\"s65\"><Data ss:Type=\"String\">Part Type</Data></Cell><Cell ss:StyleID=\"s65\"><Data ss:Type=\"String\">Position</Data></Cell><Cell ss:StyleID=\"s65\"><Data ss:Type=\"String\">Invalid Attributes</Data></Cell><Cell ss:StyleID=\"s65\"><Data ss:Type=\"String\">Notes</Data></Cell></Row>");}
		invalid_count=0; printed_header=0;
		for(i=0;i<=apps_count-1;i++)
		{
			for(j=0; j<=apps[i]->attributeCount-1; j++)
			{
				sprintNiceAttribute(attributeStrA, attributeStrB, apps[i], j);
				if(attributeStrB[0]!=0)
				{
					if(!printed_header && verbosity>2){printf("App Id\tBase vid\tPart Type\tPosition\tPart\tErrors\n"); printed_header=1;}
					if(verbosity>2){printf("%d\t%d\t%d\t%d\t%s\t%s\n",apps[i]->id,apps[i]->basevid,apps[i]->parttype,apps[i]->position,partsList[apps[i]->partsListid],attributeStrB);}
					if(assessment_filename[0])
					{
						fillNiceMMYstruct(&structMMYtemp,apps[i]->basevid);
						fprintf(assessment_fileptr,"<Row><Cell><Data ss:Type=\"Number\">%d</Data></Cell><Cell><Data ss:Type=\"String\">%s</Data></Cell><Cell><Data ss:Type=\"String\">%s</Data></Cell><Cell><Data ss:Type=\"Number\">%d</Data></Cell><Cell><Data ss:Type=\"String\">%s</Data></Cell><Cell><Data ss:Type=\"String\">%s</Data></Cell><Cell><Data ss:Type=\"String\">%s</Data></Cell><Cell><Data ss:Type=\"String\">%s</Data></Cell><Cell><Data ss:Type=\"String\">%s</Data></Cell></Row>",apps[i]->id, structMMYtemp.makeName,structMMYtemp.modelName,structMMYtemp.year,partsList[apps[i]->partsListid],sprintNicePartTypeName(strPartTypeName,apps[i]->parttype),sprintNicePositionName(strPositionName,apps[i]->position),attributeStrB,apps[i]->notes);
					}
					invalid_count++;
				}
			}
		}

		if(invalid_count){strcpy(excelTabColorXMLtag,"<TabColorIndex>10</TabColorIndex>");}
		if(assessment_filename[0]){fprintf(assessment_fileptr,"</Table><WorksheetOptions xmlns=\"urn:schemas-microsoft-com:office:excel\"><PageSetup><Header x:Margin=\"0.3\"/><Footer x:Margin=\"0.3\"/><PageMargins x:Bottom=\"0.75\" x:Left=\"0.7\" x:Right=\"0.7\" x:Top=\"0.75\"/></PageSetup>%s<FreezePanes/><FrozenNoSplit/><SplitHorizontal>1</SplitHorizontal><TopRowBottomPane>1</TopRowBottomPane><ActivePane>2</ActivePane><Panes><Pane><Number>3</Number></Pane><Pane><Number>2</Number><ActiveRow>0</ActiveRow></Pane></Panes><ProtectObjects>False</ProtectObjects><ProtectScenarios>False</ProtectScenarios></WorksheetOptions></Worksheet>",excelTabColorXMLtag);}
		if(verbosity>0){printf("Invalid vcdb codes:%d\n",invalid_count);}



		//check validity of coded attributes configurations
		if(verbosity>2){printf("checking for valid vcdb configurtions\n");}
		if(assessment_filename[0]){fprintf(assessment_fileptr,"<Worksheet ss:Name=\"Invalid Configs\"><Table ss:ExpandedColumnCount=\"9\" x:FullColumns=\"1\" x:FullRows=\"1\" ss:DefaultRowHeight=\"15\"><Column ss:AutoFitWidth=\"0\" ss:Width=\"45\"/><Column ss:AutoFitWidth=\"0\" ss:Width=\"78.75\"/><Column ss:AutoFitWidth=\"0\" ss:Width=\"99.75\"/><Column ss:AutoFitWidth=\"0\" ss:Width=\"31.5\"/><Column ss:AutoFitWidth=\"0\" ss:Width=\"60\"/><Column ss:AutoFitWidth=\"0\" ss:Width=\"112.5\"/><Column ss:Width=\"43.5\"/><Column ss:AutoFitWidth=\"0\" ss:Width=\"237\"/><Column ss:AutoFitWidth=\"0\" ss:Width=\"319.5\"/><Row><Cell ss:StyleID=\"s65\"><Data ss:Type=\"String\">App Id</Data></Cell><Cell ss:StyleID=\"s65\"><Data ss:Type=\"String\">Make</Data></Cell><Cell ss:StyleID=\"s65\"><Data ss:Type=\"String\">Model</Data></Cell><Cell ss:StyleID=\"s65\"><Data ss:Type=\"String\">Year</Data></Cell><Cell ss:StyleID=\"s65\"><Data ss:Type=\"String\">Part</Data></Cell><Cell ss:StyleID=\"s65\"><Data ss:Type=\"String\">Part Type</Data></Cell><Cell ss:StyleID=\"s65\"><Data ss:Type=\"String\">Position</Data></Cell><Cell ss:StyleID=\"s65\"><Data ss:Type=\"String\">Invalid Attributes Combination</Data></Cell><Cell ss:StyleID=\"s65\"><Data ss:Type=\"String\">Notes</Data></Cell></Row>"); excelTabColorXMLtag[0]=0;}
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
						printf("%d\t%d\t%d\t%d\t%s\t%s\t%s\n",apps[i]->id,apps[i]->basevid,apps[i]->parttype,apps[i]->position,partsList[apps[i]->partsListid],attributeStrA,attributeStrB);
					}

					if(assessment_filename[0]){fillNiceMMYstruct(&structMMYtemp,apps[i]->basevid); fprintf(assessment_fileptr,"<Row><Cell><Data ss:Type=\"Number\">%d</Data></Cell><Cell><Data ss:Type=\"String\">%s</Data></Cell><Cell><Data ss:Type=\"String\">%s</Data></Cell><Cell><Data ss:Type=\"Number\">%d</Data></Cell><Cell><Data ss:Type=\"String\">%s</Data></Cell><Cell><Data ss:Type=\"String\">%s</Data></Cell><Cell><Data ss:Type=\"String\">%s</Data></Cell><Cell><Data ss:Type=\"String\">%s</Data></Cell><Cell><Data ss:Type=\"String\">%s</Data></Cell></Row>",apps[i]->id,structMMYtemp.makeName,structMMYtemp.modelName,structMMYtemp.year,partsList[apps[i]->partsListid],sprintNicePartTypeName(strPartTypeName,apps[i]->parttype),sprintNicePositionName(strPositionName,apps[i]->position),sprintNiceQualifiers(attributeStrA, attributeStrB, apps[i]),apps[i]->notes);}
					invalid_count++;
				}
			}
		}
		if(invalid_count){strcpy(excelTabColorXMLtag,"<TabColorIndex>13</TabColorIndex>");}
		if(assessment_filename[0]){fprintf(assessment_fileptr,"</Table><WorksheetOptions xmlns=\"urn:schemas-microsoft-com:office:excel\"><PageSetup><Header x:Margin=\"0.3\"/><Footer x:Margin=\"0.3\"/><PageMargins x:Bottom=\"0.75\" x:Left=\"0.7\" x:Right=\"0.7\" x:Top=\"0.75\"/></PageSetup>%s<FreezePanes/><FrozenNoSplit/><SplitHorizontal>1</SplitHorizontal><TopRowBottomPane>1</TopRowBottomPane><ActivePane>2</ActivePane><Panes><Pane><Number>3</Number></Pane><Pane><Number>2</Number><ActiveRow>0</ActiveRow></Pane></Panes><ProtectObjects>False</ProtectObjects><ProtectScenarios>False</ProtectScenarios></WorksheetOptions></Worksheet>",excelTabColorXMLtag);}
		if(verbosity>0){printf("invalid vcdb configurations:%d\n",invalid_count);}
	}

#endif

	//check for duplicates
	if(verbosity>2){printf("checking for duplicates\n");}
	if(assessment_filename[0]){fprintf(assessment_fileptr,"<Worksheet ss:Name=\"Duplicates\"><Table ss:ExpandedColumnCount=\"9\" x:FullColumns=\"1\" x:FullRows=\"1\" ss:DefaultRowHeight=\"15\"><Column ss:AutoFitWidth=\"0\" ss:Width=\"45\"/><Column ss:AutoFitWidth=\"0\" ss:Width=\"78.75\"/><Column ss:AutoFitWidth=\"0\" ss:Width=\"99.75\"/><Column ss:Width=\"31.5\"/><Column ss:AutoFitWidth=\"0\" ss:Width=\"60\"/><Column ss:AutoFitWidth=\"0\" ss:Width=\"112.5\"/><Column ss:AutoFitWidth=\"0\" ss:Width=\"75\"/><Column ss:AutoFitWidth=\"0\" ss:Width=\"237\"/><Column ss:AutoFitWidth=\"0\" ss:Width=\"319.5\"/><Row><Cell ss:StyleID=\"s65\"><Data ss:Type=\"String\">App Id</Data></Cell><Cell ss:StyleID=\"s65\"><Data ss:Type=\"String\">Make</Data></Cell><Cell ss:StyleID=\"s65\"><Data ss:Type=\"String\">Model</Data></Cell><Cell ss:StyleID=\"s65\"><Data ss:Type=\"String\">Year</Data></Cell><Cell ss:StyleID=\"s65\"><Data ss:Type=\"String\">Part</Data></Cell><Cell ss:StyleID=\"s65\"><Data ss:Type=\"String\">Part Type</Data></Cell><Cell ss:StyleID=\"s65\"><Data ss:Type=\"String\">Position</Data></Cell><Cell ss:StyleID=\"s65\"><Data ss:Type=\"String\">VCdb Attributes</Data></Cell><Cell ss:StyleID=\"s65\"><Data ss:Type=\"String\">Notes</Data></Cell></Row>"); excelTabColorXMLtag[0]=0;}
	invalid_count=0; printed_header=0;
	for(i=0;i<=apps_count-2;i++)
	{
		attributeStrA[0]=0; attributeStrB[0]=0;
		for(j=0; j<=apps[i]->attributeCount-1; j++){snprintf(strTemp,1024,"%s:%d;",sprintVCdbAtrributeFromToken(attributeNameTemp,apps[i]->attributeTokens[j]),apps[i]->attributeValues[j]); strcat(attributeStrA,strTemp);} strcat(attributeStrA,apps[i]->notes);
		for(j=0; j<=apps[i+1]->attributeCount-1; j++){snprintf(strTemp,1024,"%s:%d;",sprintVCdbAtrributeFromToken(attributeNameTemp,apps[i+1]->attributeTokens[j]),apps[i+1]->attributeValues[j]); strcat(attributeStrB,strTemp);} strcat(attributeStrB,apps[i+1]->notes);
		if(apps[i]->basevid==apps[i+1]->basevid && apps[i]->parttype==apps[i+1]->parttype && apps[i]->position==apps[i+1]->position && apps[i]->mfrlabelid==apps[i+1]->mfrlabelid && apps[i]->partsListid==apps[i+1]->partsListid && strcmp(attributeStrA,attributeStrB)==0 && apps[i]->assetid==apps[i+1]->assetid && apps[i]->assetorder==apps[i+1]->assetorder)
		{
#ifdef WITH_MYSQL
			if(vcdb_used)
			{
				if(verbosity>2)
				{
					if(!printed_header){printf("App id\tMake\tModel\tYear\tBase vid\tPart Type\tPosition\tPart\tQualifiers\n"); printed_header=1;}
					printf("%d\t%s\t%d\t%d\t%d\t%s\t%s\n",apps[i]->id,sprintNiceMMY(niceMMYtemp,apps[i]->basevid, 9),apps[i]->basevid,apps[i]->parttype,apps[i]->position,partsList[apps[i]->partsListid],attributeStrA);
					printf("%d\t%s\t%d\t%d\t%d\t%s\t%s\n",apps[i+1]->id,sprintNiceMMY(niceMMYtemp,apps[i+1]->basevid, 9),apps[i+1]->basevid,apps[i+1]->parttype,apps[i+1]->position,partsList[apps[i+1]->partsListid],attributeStrA);
				}

				if(assessment_filename[0])
				{
					fillNiceMMYstruct(&structMMYtemp,apps[i]->basevid);
					fprintf(assessment_fileptr,"<Row><Cell><Data ss:Type=\"Number\">%d</Data></Cell><Cell><Data ss:Type=\"String\">%s</Data></Cell><Cell><Data ss:Type=\"String\">%s</Data></Cell><Cell><Data ss:Type=\"Number\">%d</Data></Cell><Cell><Data ss:Type=\"String\">%s</Data></Cell><Cell><Data ss:Type=\"String\">%s</Data></Cell><Cell><Data ss:Type=\"String\">%s</Data></Cell><Cell><Data ss:Type=\"String\">%s</Data></Cell><Cell><Data ss:Type=\"String\">%s</Data></Cell></Row>",apps[i]->id,structMMYtemp.makeName,structMMYtemp.modelName,structMMYtemp.year,partsList[apps[i]->partsListid],sprintNicePartTypeName(strPartTypeName,apps[i]->parttype),sprintNicePositionName(strPositionName,apps[i]->position),sprintNiceQualifiers(attributeStrA, attributeStrB, apps[i]),apps[i]->notes);
					fprintf(assessment_fileptr,"<Row><Cell><Data ss:Type=\"Number\">%d</Data></Cell><Cell><Data ss:Type=\"String\">%s</Data></Cell><Cell><Data ss:Type=\"String\">%s</Data></Cell><Cell><Data ss:Type=\"Number\">%d</Data></Cell><Cell><Data ss:Type=\"String\">%s</Data></Cell><Cell><Data ss:Type=\"String\">%s</Data></Cell><Cell><Data ss:Type=\"String\">%s</Data></Cell><Cell><Data ss:Type=\"String\">%s</Data></Cell><Cell><Data ss:Type=\"String\">%s</Data></Cell></Row>",apps[i+1]->id,structMMYtemp.makeName,structMMYtemp.modelName,structMMYtemp.year,partsList[apps[i]->partsListid],sprintNicePartTypeName(strPartTypeName,apps[i]->parttype),sprintNicePositionName(strPositionName,apps[i]->position),sprintNiceQualifiers(attributeStrA, attributeStrB, apps[i]),apps[i]->notes);
					fprintf(assessment_fileptr,"<Row><Cell><Data ss:Type=\"String\"></Data></Cell><Cell><Data ss:Type=\"String\"></Data></Cell><Cell><Data ss:Type=\"String\"></Data></Cell><Cell><Data ss:Type=\"String\"></Data></Cell><Cell><Data ss:Type=\"String\"></Data></Cell><Cell><Data ss:Type=\"String\"></Data></Cell><Cell><Data ss:Type=\"String\"></Data></Cell><Cell><Data ss:Type=\"String\"></Data></Cell><Cell><Data ss:Type=\"String\"></Data></Cell></Row>");
				}
			}
			else
			{// no database connection was specified 

				if(verbosity>2)
				{
					if(!printed_header){printf("App id\tBase vid\tPart Type\tPosition\tPart\tQualifiers\n"); printed_header=1;}
					printf("%d\t%d\t%d\t%d\t%s\t%s\n",apps[i]->id,apps[i]->basevid,apps[i]->parttype,apps[i]->position,partsList[apps[i]->partsListid],attributeStrA);
					printf("%d\t%d\t%d\t%d\t%s\t%s\n",apps[i+1]->id,apps[i+1]->basevid,apps[i+1]->parttype,apps[i+1]->position,partsList[apps[i+1]->partsListid],attributeStrA);
				}
			}
#else
			if(verbosity>2)
			{
				if(!printed_header){printf("App id\tBase vid\tPart Type\tPosition\tPart\tQualifiers\n"); printed_header=1;}
				printf("%d\t%d\t%d\t%d\t%s\t%s\n",apps[i]->id,apps[i]->basevid,apps[i]->parttype,apps[i]->position,partsList[apps[i]->partsListid],attributeStrA);
				printf("%d\t%d\t%d\t%d\t%s\t%s\n",apps[i+1]->id,apps[i+1]->basevid,apps[i+1]->parttype,apps[i+1]->position,partsList[apps[i+1]->partsListid],attributeStrA);
			}
#endif
			invalid_count++;
		}
	}
	if(invalid_count){strcpy(excelTabColorXMLtag,"<TabColorIndex>13</TabColorIndex>");}
	if(assessment_filename[0]){fprintf(assessment_fileptr,"</Table><WorksheetOptions xmlns=\"urn:schemas-microsoft-com:office:excel\"><PageSetup><Header x:Margin=\"0.3\"/><Footer x:Margin=\"0.3\"/><PageMargins x:Bottom=\"0.75\" x:Left=\"0.7\" x:Right=\"0.7\" x:Top=\"0.75\"/></PageSetup>%s<FreezePanes/><FrozenNoSplit/><SplitHorizontal>1</SplitHorizontal><TopRowBottomPane>1</TopRowBottomPane><ActivePane>2</ActivePane><Panes><Pane><Number>3</Number></Pane><Pane><Number>2</Number><ActiveRow>0</ActiveRow></Pane></Panes><ProtectObjects>False</ProtectObjects><ProtectScenarios>False</ProtectScenarios></WorksheetOptions></Worksheet>",excelTabColorXMLtag);}
	if(verbosity>0){printf("Duplicate apps:%d\n",invalid_count);}


	//check for overlaps
	if(verbosity>2){printf("checking for overlaps\n");}
	if(assessment_filename[0]){fprintf(assessment_fileptr,"<Worksheet ss:Name=\"Overlaps\"><Table ss:ExpandedColumnCount=\"12\" x:FullColumns=\"1\" x:FullRows=\"1\" ss:DefaultRowHeight=\"15\"><Column ss:AutoFitWidth=\"0\" ss:Width=\"45\"/><Column ss:AutoFitWidth=\"0\" ss:Width=\"45\"/><Column ss:AutoFitWidth=\"0\" ss:Width=\"78.75\"/><Column ss:AutoFitWidth=\"0\" ss:Width=\"99.75\"/><Column ss:AutoFitWidth=\"0\" ss:Width=\"31.5\"/><Column ss:AutoFitWidth=\"0\" ss:Width=\"60\" ss:Span=\"1\"/><Column ss:Index=\"8\" ss:AutoFitWidth=\"0\" ss:Width=\"112.5\"/><Column ss:AutoFitWidth=\"0\" ss:Width=\"75\"/><Column ss:AutoFitWidth=\"0\" ss:Width=\"237\"/><Column ss:AutoFitWidth=\"0\" ss:Width=\"319.5\"/><Row><Cell ss:StyleID=\"s65\"><Data ss:Type=\"String\">App A id</Data></Cell><Cell ss:StyleID=\"s65\"><Data ss:Type=\"String\">App B id</Data></Cell><Cell ss:StyleID=\"s65\"><Data ss:Type=\"String\">Make</Data></Cell><Cell ss:StyleID=\"s65\"><Data ss:Type=\"String\">Model</Data></Cell><Cell ss:StyleID=\"s65\"><Data ss:Type=\"String\">Year</Data></Cell><Cell ss:StyleID=\"s65\"><Data ss:Type=\"String\">Part A</Data></Cell><Cell ss:StyleID=\"s65\"><Data ss:Type=\"String\">Part B</Data></Cell><Cell ss:StyleID=\"s65\"><Data ss:Type=\"String\">Part Type</Data></Cell><Cell ss:StyleID=\"s65\"><Data ss:Type=\"String\">Position</Data></Cell><Cell ss:StyleID=\"s65\"><Data ss:Type=\"String\">VCdb Attributes</Data></Cell><Cell ss:StyleID=\"s65\"><Data ss:Type=\"String\">Notes</Data></Cell></Row>"); excelTabColorXMLtag[0]=0;}

	invalid_count=0; printed_header=0;
	for(i=0;i<=apps_count-2;i++)
	{
		attributeStrA[0]=0; attributeStrB[0]=0;
		for(j=0; j<=apps[i]->attributeCount-1; j++){snprintf(strTemp,1024,"%s:%d;",sprintVCdbAtrributeFromToken(attributeNameTemp,apps[i]->attributeTokens[j]),apps[i]->attributeValues[j]); strcat(attributeStrA,strTemp);} strcat(attributeStrA,apps[i]->notes);
		for(j=0; j<=apps[i+1]->attributeCount-1; j++){snprintf(strTemp,1024,"%s:%d;",sprintVCdbAtrributeFromToken(attributeNameTemp,apps[i+1]->attributeTokens[j]),apps[i+1]->attributeValues[j]); strcat(attributeStrB,strTemp);} strcat(attributeStrB,apps[i+1]->notes);
		if(apps[i]->basevid==apps[i+1]->basevid && apps[i]->parttype==apps[i+1]->parttype && apps[i]->position==apps[i+1]->position && apps[i]->mfrlabelid==apps[i+1]->mfrlabelid && apps[i]->partsListid!=apps[i+1]->partsListid && strcmp(attributeStrA,attributeStrB)==0 && apps[i]->assetid==apps[i+1]->assetid && apps[i]->assetorder==apps[i+1]->assetorder)
		{
#ifdef WITH_MYSQL
			if(vcdb_used)
			{
				if(verbosity>2)
				{
					if(!printed_header){printf("Make\tModel\tYear\tPart Type\tPosition\tPart A\tQualifiers A\tPart B\tQualifiers B\n"); printed_header=1;}
					printf("%s\t%d\t%d\t%d\t%s\t%s\t%s\t%s\n",sprintNiceMMY(niceMMYtemp,apps[i]->basevid, 9),apps[i]->parttype,apps[i]->position,partsList[apps[i]->partsListid],attributeStrA,partsList[apps[i+1]->partsListid],attributeStrB);
				}
				if(assessment_filename[0]){fillNiceMMYstruct(&structMMYtemp,apps[i]->basevid); fprintf(assessment_fileptr,"<Row><Cell><Data ss:Type=\"Number\">%d</Data></Cell><Cell><Data ss:Type=\"Number\">%d</Data></Cell><Cell><Data ss:Type=\"String\">%s</Data></Cell><Cell><Data ss:Type=\"String\">%s</Data></Cell><Cell><Data ss:Type=\"Number\">%d</Data></Cell><Cell><Data ss:Type=\"String\">%s</Data></Cell><Cell><Data ss:Type=\"String\">%s</Data></Cell><Cell><Data ss:Type=\"String\">%s</Data></Cell><Cell><Data ss:Type=\"String\">%s</Data></Cell><Cell><Data ss:Type=\"String\">%s</Data></Cell><Cell><Data ss:Type=\"String\">%s</Data></Cell></Row>",apps[i]->id,apps[i+1]->id,structMMYtemp.makeName,structMMYtemp.modelName,structMMYtemp.year,partsList[apps[i]->partsListid],partsList[apps[i+1]->partsListid],sprintNicePartTypeName(strPartTypeName,apps[i]->parttype),sprintNicePositionName(strPositionName,apps[i]->position),sprintNiceQualifiers(attributeStrA, attributeStrB, apps[i]),apps[i]->notes);}
			}
			else
			{// no database was specified
				if(verbosity>2)
				{
					if(!printed_header){printf("Base vid A\tPart Type\tPosition\tPart A\tQualifiers A\tPart B\tQualifiers B\n"); printed_header=1;}
					printf("%d\t%d\t%d\t%s\t%s\t%s\t%s\n",apps[i]->basevid,apps[i]->parttype,apps[i]->position,partsList[apps[i]->partsListid],attributeStrA,partsList[apps[i+1]->partsListid],attributeStrB);
				}
			}
#else
			if(verbosity>2)
			{
				if(!printed_header){printf("Base vid A\tPart Type\tPosition\tPart A\tQualifiers A\tPart B\tQualifiers B\n"); printed_header=1;}
				printf("%d\t%d\t%d\t%s\t%s\t%s\t%s\n",apps[i]->basevid,apps[i]->parttype,apps[i]->position,partsList[apps[i]->partsListid],attributeStrA,partsList[apps[i+1]->partsListid],attributeStrB);
			}
#endif
			invalid_count++;
		}
	}
	if(invalid_count){strcpy(excelTabColorXMLtag,"<TabColorIndex>13</TabColorIndex>");}
	if(assessment_filename[0]){fprintf(assessment_fileptr,"</Table><WorksheetOptions xmlns=\"urn:schemas-microsoft-com:office:excel\"><PageSetup><Header x:Margin=\"0.3\"/><Footer x:Margin=\"0.3\"/><PageMargins x:Bottom=\"0.75\" x:Left=\"0.7\" x:Right=\"0.7\" x:Top=\"0.75\"/></PageSetup>%s<FreezePanes/><FrozenNoSplit/><SplitHorizontal>1</SplitHorizontal><TopRowBottomPane>1</TopRowBottomPane><ActivePane>2</ActivePane><Panes><Pane><Number>3</Number></Pane><Pane><Number>2</Number><ActiveRow>0</ActiveRow></Pane></Panes><ProtectObjects>False</ProtectObjects><ProtectScenarios>False</ProtectScenarios></WorksheetOptions></Worksheet>",excelTabColorXMLtag);}
	if(verbosity>0){printf("Overlaps:%d\n",invalid_count);}



	//check for comment-no-comment errors
	if(verbosity>2){printf("checking for CNCs...\n");}
	if(assessment_filename[0]){fprintf(assessment_fileptr,"<Worksheet ss:Name=\"CNC Overlaps\"><Table ss:ExpandedColumnCount=\"9\" x:FullColumns=\"1\" x:FullRows=\"1\" ss:DefaultRowHeight=\"15\"><Column ss:AutoFitWidth=\"0\" ss:Width=\"45\"/><Column ss:AutoFitWidth=\"0\" ss:Width=\"78.75\"/><Column ss:AutoFitWidth=\"0\" ss:Width=\"99.75\"/><Column ss:AutoFitWidth=\"0\" ss:Width=\"31.5\"/><Column ss:AutoFitWidth=\"0\" ss:Width=\"60\"/><Column ss:AutoFitWidth=\"0\" ss:Width=\"112.5\"/><Column ss:AutoFitWidth=\"0\" ss:Width=\"75\"/><Column ss:AutoFitWidth=\"0\" ss:Width=\"237\"/><Column ss:AutoFitWidth=\"0\" ss:Width=\"244.5\"/><Row><Cell ss:StyleID=\"s65\"><Data ss:Type=\"String\">App Id</Data></Cell><Cell ss:StyleID=\"s65\"><Data ss:Type=\"String\">Make</Data></Cell><Cell ss:StyleID=\"s65\"><Data ss:Type=\"String\">Model</Data></Cell><Cell ss:StyleID=\"s65\"><Data ss:Type=\"String\">Year</Data></Cell><Cell ss:StyleID=\"s65\"><Data ss:Type=\"String\">Part</Data></Cell><Cell ss:StyleID=\"s65\"><Data ss:Type=\"String\">Part Type</Data></Cell><Cell ss:StyleID=\"s65\"><Data ss:Type=\"String\">Position</Data></Cell><Cell ss:StyleID=\"s65\"><Data ss:Type=\"String\">VCdb Attributes</Data></Cell><Cell ss:StyleID=\"s65\"><Data ss:Type=\"String\">Notes</Data></Cell></Row>"); excelTabColorXMLtag[0]=0;}


#ifdef WITH_MYSQL
	if(verbosity>2){if(vcdb_used){printf("Make\tModel\tYear\tBase vid A\tPart Type\tPosition\tPart A\tQualifiers A\tPart B\tQualifiers B\n");}else{printf("Base vid A\tPart Type\tPosition\tPart A\tQualifiers A\tPart B\tQualifiers B\n");}}
#else
	if(verbosity>2){printf("Base vid A\tPart Type\tPosition\tPart A\tQualifiers A\tPart B\tQualifiers B\n");}
#endif
	invalid_count=0;
	for(i=0;i<=apps_count-2;i++)
	{
		attributeStrA[0]=0; attributeStrB[0]=0;
		for(j=0; j<=apps[i]->attributeCount-1; j++){snprintf(strTemp,1024,"%s:%d;",sprintVCdbAtrributeFromToken(attributeNameTemp,apps[i]->attributeTokens[j]),apps[i]->attributeValues[j]); strcat(attributeStrA,strTemp);} strcat(attributeStrA,apps[i]->notes);
		for(j=0; j<=apps[i+1]->attributeCount-1; j++){snprintf(strTemp,1024,"%s:%d;",sprintVCdbAtrributeFromToken(attributeNameTemp,apps[i+1]->attributeTokens[j]),apps[i+1]->attributeValues[j]); strcat(attributeStrB,strTemp);} strcat(attributeStrB,apps[i+1]->notes);
		if(apps[i]->basevid==apps[i+1]->basevid && apps[i]->parttype==apps[i+1]->parttype && apps[i]->position==apps[i+1]->position && apps[i]->mfrlabelid==apps[i+1]->mfrlabelid &&((strlen(attributeStrA)==0 && strlen(attributeStrB)!=0)||(strlen(attributeStrA)!=0 && strlen(attributeStrB)==0)))
		{
#ifdef WITH_MYSQL
			if(vcdb_used)
			{
				if(verbosity>2)
				{
					printf("%s\t%d\t%d\t%d\t%s\t%s\t%s\t%s\n",sprintNiceMMY(niceMMYtemp,apps[i]->basevid, 9),apps[i]->basevid,apps[i]->parttype,apps[i]->position,partsList[apps[i]->partsListid],attributeStrA,partsList[apps[i+1]->partsListid],attributeStrB);
				}

				if(assessment_filename[0])
				{
					fillNiceMMYstruct(&structMMYtemp,apps[i]->basevid);
					fprintf(assessment_fileptr,"<Row><Cell><Data ss:Type=\"Number\">%d</Data></Cell><Cell><Data ss:Type=\"String\">%s</Data></Cell><Cell><Data ss:Type=\"String\">%s</Data></Cell><Cell><Data ss:Type=\"Number\">%d</Data></Cell><Cell><Data ss:Type=\"String\">%s</Data></Cell><Cell><Data ss:Type=\"String\">%s</Data></Cell><Cell><Data ss:Type=\"String\">%s</Data></Cell><Cell><Data ss:Type=\"String\">%s</Data></Cell><Cell><Data ss:Type=\"String\">%s</Data></Cell></Row>",apps[i]->id,structMMYtemp.makeName,structMMYtemp.modelName,structMMYtemp.year,partsList[apps[i]->partsListid],sprintNicePartTypeName(strPartTypeName,apps[i]->parttype),sprintNicePositionName(strPositionName,apps[i]->position),sprintNiceQualifiers(attributeStrA, attributeStrB, apps[i]),apps[i]->notes);
					fprintf(assessment_fileptr,"<Row><Cell><Data ss:Type=\"Number\">%d</Data></Cell><Cell><Data ss:Type=\"String\">%s</Data></Cell><Cell><Data ss:Type=\"String\">%s</Data></Cell><Cell><Data ss:Type=\"Number\">%d</Data></Cell><Cell><Data ss:Type=\"String\">%s</Data></Cell><Cell><Data ss:Type=\"String\">%s</Data></Cell><Cell><Data ss:Type=\"String\">%s</Data></Cell><Cell><Data ss:Type=\"String\">%s</Data></Cell><Cell><Data ss:Type=\"String\">%s</Data></Cell></Row>",apps[i+1]->id,structMMYtemp.makeName,structMMYtemp.modelName,structMMYtemp.year,partsList[apps[i+1]->partsListid],sprintNicePartTypeName(strPartTypeName,apps[i+1]->parttype),sprintNicePositionName(strPositionName,apps[i+1]->position),sprintNiceQualifiers(attributeStrA, attributeStrB, apps[i+1]),apps[i+1]->notes);
					fprintf(assessment_fileptr,"<Row><Cell><Data ss:Type=\"String\"></Data></Cell><Cell><Data ss:Type=\"String\"></Data></Cell><Cell><Data ss:Type=\"String\"></Data></Cell><Cell><Data ss:Type=\"String\"></Data></Cell><Cell><Data ss:Type=\"String\"></Data></Cell><Cell><Data ss:Type=\"String\"></Data></Cell><Cell><Data ss:Type=\"String\"></Data></Cell><Cell><Data ss:Type=\"String\"></Data></Cell><Cell><Data ss:Type=\"String\"></Data></Cell></Row>");
				}

			}
			else
			{// no database was spefified
				printf("%d\t%d\t%d\t%s\t%s\t%s\t%s\n",apps[i]->basevid,apps[i]->parttype,apps[i]->position,partsList[apps[i]->partsListid],attributeStrA,partsList[apps[i+1]->partsListid],attributeStrB);
			}
#else
			if(verbosity>2){printf("%d\t%d\t%d\t%s\t%s\t%s\t%s\n",apps[i]->basevid,apps[i]->parttype,apps[i]->position,partsList[apps[i]->partsListid],attributeStrA,partsList[apps[i+1]->partsListid],attributeStrB);}
#endif
			invalid_count++;
		}
	}
	if(invalid_count){strcpy(excelTabColorXMLtag,"<TabColorIndex>13</TabColorIndex>");}
	if(assessment_filename[0]){fprintf(assessment_fileptr,"</Table><WorksheetOptions xmlns=\"urn:schemas-microsoft-com:office:excel\"><PageSetup><Header x:Margin=\"0.3\"/><Footer x:Margin=\"0.3\"/><PageMargins x:Bottom=\"0.75\" x:Left=\"0.7\" x:Right=\"0.7\" x:Top=\"0.75\"/></PageSetup>%s<FreezePanes/><FrozenNoSplit/><SplitHorizontal>1</SplitHorizontal><TopRowBottomPane>1</TopRowBottomPane><ActivePane>2</ActivePane><Panes><Pane><Number>3</Number></Pane><Pane><Number>2</Number><ActiveRow>0</ActiveRow></Pane></Panes><ProtectObjects>False</ProtectObjects><ProtectScenarios>False</ProtectScenarios></WorksheetOptions></Worksheet>",excelTabColorXMLtag);}
	if(verbosity>0){printf("CNC overlaps:%d\n",invalid_count);}

#ifdef WITH_MYSQL
	// check validity for parttype-position combinations

	if(pcdb_used)
	{
		if(assessment_filename[0]){fprintf(assessment_fileptr,"<Worksheet ss:Name=\"Parttype-Position\"><Table ss:ExpandedColumnCount=\"9\" x:FullColumns=\"1\" x:FullRows=\"1\" ss:DefaultRowHeight=\"15\"><Column ss:AutoFitWidth=\"0\" ss:Width=\"45\"/><Column ss:AutoFitWidth=\"0\" ss:Width=\"78\"/><Column ss:AutoFitWidth=\"0\" ss:Width=\"99.75\"/><Column ss:AutoFitWidth=\"0\" ss:Width=\"31.5\"/><Column ss:AutoFitWidth=\"0\" ss:Width=\"60\"/><Column ss:AutoFitWidth=\"0\" ss:Width=\"112.5\"/><Column ss:AutoFitWidth=\"0\" ss:Width=\"75\"/><Column ss:AutoFitWidth=\"0\" ss:Width=\"237\"/><Column ss:AutoFitWidth=\"0\" ss:Width=\"244.5\"/><Row><Cell ss:StyleID=\"s65\"><Data ss:Type=\"String\">App Id</Data></Cell><Cell ss:StyleID=\"s65\"><Data ss:Type=\"String\">Make</Data></Cell><Cell ss:StyleID=\"s65\"><Data ss:Type=\"String\">Model</Data></Cell><Cell ss:StyleID=\"s65\"><Data ss:Type=\"String\">Year</Data></Cell><Cell ss:StyleID=\"s65\"><Data ss:Type=\"String\">Part</Data></Cell><Cell ss:StyleID=\"s65\"><Data ss:Type=\"String\">Part Type</Data></Cell><Cell ss:StyleID=\"s65\"><Data ss:Type=\"String\">Position</Data></Cell><Cell ss:StyleID=\"s65\"><Data ss:Type=\"String\">VCdb Attributes</Data></Cell><Cell ss:StyleID=\"s65\"><Data ss:Type=\"String\">Notes</Data></Cell></Row>"); excelTabColorXMLtag[0]=0;}
		invalid_count=0; printed_header=0;
		for(i=0;i<=apps_count-1;i++)
		{
			sprintf(sql_command,"select codemasterid from codemaster where partterminologyid =%d and positionid =%d;",apps[i]->parttype ,apps[i]->position ); if(mysql_query(&dbPCDB,sql_command)){printf("\nSQL Error\n%s\n",sql_command);exit(1);} dbPCDBRecset = mysql_store_result(&dbPCDB);
			dbPCDBRow = mysql_fetch_row(dbPCDBRecset);
			mysql_free_result(dbPCDBRecset);
			if(dbPCDBRow == NULL)
			{// this combo of part type and position was not found in the pcdb codemaster
				if(vcdb_used)
				{
					if(verbosity>2)
					{
						if(!printed_header){printf("Make\tModel\tYear\tPart Type\tPosition\tPart\tQualifiers\n"); printed_header=1;}
						sprintNiceQualifiers(attributeStrA, attributeStrB, apps[i]);
						printf("%s\t%s\t%s\t%s\t%s\n",sprintNiceMMY(niceMMYtemp,apps[i]->basevid, 9),sprintNicePartTypeName(strPartTypeName,apps[i]->parttype),sprintNicePositionName(strPositionName,apps[i]->position),partsList[apps[i]->partsListid],attributeStrA);
					}
					fillNiceMMYstruct(&structMMYtemp,apps[i]->basevid);
					if(assessment_filename[0]){fprintf(assessment_fileptr,"<Row><Cell><Data ss:Type=\"Number\">%d</Data></Cell><Cell><Data ss:Type=\"String\">%s</Data></Cell><Cell><Data ss:Type=\"String\">%s</Data></Cell><Cell><Data ss:Type=\"Number\">%d</Data></Cell><Cell><Data ss:Type=\"String\">%s</Data></Cell><Cell><Data ss:Type=\"String\">%s</Data></Cell><Cell><Data ss:Type=\"String\">%s</Data></Cell><Cell><Data ss:Type=\"String\">%s</Data></Cell><Cell><Data ss:Type=\"String\">%s</Data></Cell></Row>",apps[i]->id,structMMYtemp.makeName,structMMYtemp.modelName,structMMYtemp.year,partsList[apps[i]->partsListid],sprintNicePartTypeName(strPartTypeName,apps[i]->parttype),sprintNicePositionName(strPositionName,apps[i]->position),sprintNiceQualifiers(attributeStrA, attributeStrB, apps[i]),apps[i]->notes);}
				}
				else
				{// no vcdb database connection was specified 
					if(!printed_header){printf("Base vid\tPart Type\tPosition\tPart\tQualifiers\n"); printed_header=1;}
					printf("%d\t%s\t%s\t%s\t%s\n",apps[i]->basevid,sprintNicePartTypeName(strPartTypeName,apps[i]->parttype),sprintNicePositionName(strPositionName,apps[i]->position),partsList[apps[i]->partsListid],attributeStrA);
				}
				invalid_count++;
			}
		}
		if(invalid_count){strcpy(excelTabColorXMLtag,"<TabColorIndex>13</TabColorIndex>");}
		if(assessment_filename[0]){fprintf(assessment_fileptr,"</Table><WorksheetOptions xmlns=\"urn:schemas-microsoft-com:office:excel\"><PageSetup><Header x:Margin=\"0.3\"/><Footer x:Margin=\"0.3\"/><PageMargins x:Bottom=\"0.75\" x:Left=\"0.7\" x:Right=\"0.7\" x:Top=\"0.75\"/></PageSetup>%s<FreezePanes/><FrozenNoSplit/><SplitHorizontal>1</SplitHorizontal><TopRowBottomPane>1</TopRowBottomPane><ActivePane>2</ActivePane><Panes><Pane><Number>3</Number></Pane><Pane><Number>2</Number><ActiveRow>0</ActiveRow></Pane></Panes><ProtectObjects>False</ProtectObjects><ProtectScenarios>False</ProtectScenarios></WorksheetOptions></Worksheet>",excelTabColorXMLtag);}
		if(verbosity>0){printf("Parttype-position violations:%d\n",invalid_count);}
	}
#endif

	if(extract_parttypes)
	{
		for(j=0;j<=parttypeListCount-1;j++)
		{
			if(pcdb_used)
			{
				sprintNicePartTypeName(attributeStrA,parttypeList[j]);
				printf("%d\t%s\n",parttypeList[j],attributeStrA);
			}else{printf("%d\n",parttypeList[j]);}
		}
	}

	if(extract_parts){for(j=0;j<=partsListCount-1;j++){printf("%s\n",partsList[j]);}}
	if(extract_mfrlabels){for(j=0;j<=mfrlabelListCount-1;j++){printf("%s\n",mfrlabelList[j]);}}
	if(extract_assets){for(j=0;j<=assetListCount-1;j++){printf("%s\n",assetList[j]);}}

/*
	if(extract_partassetlinks)
	{
		for(i=0;i<=apps_count-1;i++)
		{
			if(apps[i]->local_asset_index>=0)
			{
				found=0;
				for(j=0;j<=part_asset_connections_count-1;j++)
				{
					if(apps[i]->local_part_index == part_asset_connections[0][j] &&  apps[i]->local_asset_index == part_asset_connections[1][j]          ){found=1; break;}
				}
				if(part_asset_connections_count>99999){printf("too many part-asset connections\n"); exit(1);}
				if(!found){part_asset_connections[0][part_asset_connections_count] = apps[i]->local_part_index; part_asset_connections[1][part_asset_connections_count] = apps[i]->local_asset_index; part_asset_connections_count++;}
			}
		}
		for(j=0;j<=part_asset_connections_count-1;j++){printf("%s,%s\n", extractedPartList[part_asset_connections[0][j]],assetList[part_asset_connections[1][j]]);}
	}
*/
	if(flatten>0)
	{
		switch(flatten)
		{
			case 1:
			//coded values only (no human -readable)
				if(assetsFoundInXML)
				{// 1 or more asset tags were found in input xml file
					printf("basevid\tpart\tparttypeid\tpositionid\tquantity\tqualifers\tasset\tasset_item_order\tnotes\r\n");
					for(i=0;i<=apps_count-1;i++)
					{
						attributeStrA[0]=0; for(j=0; j<=apps[i]->attributeCount-1; j++){sprintf(strTemp,"%s:%d;",sprintVCdbAtrributeFromToken(attributeNameTemp,apps[i]->attributeTokens[j]),apps[i]->attributeValues[j]); strcat(attributeStrA,strTemp);}
						printf("%d\t%s\t%d\t%d\t%d\t%s\t%s\t%d\t%s\r\n",apps[i]->basevid,partsList[apps[i]->partsListid],apps[i]->parttype,apps[i]->position,apps[i]->qty,attributeStrA,assetList[apps[i]->assetid],apps[i]->assetorder,apps[i]->notes);
					}
				}
				else
				{// no asset tags were found in input xml - export without asset and asset-order columns
					printf("basevid\tpart\tparttypeid\tpositionid\tquantity\tqualifers\tnotes\r\n");
					for(i=0;i<=apps_count-1;i++)
					{
						attributeStrA[0]=0; for(j=0; j<=apps[i]->attributeCount-1; j++){sprintf(strTemp,"%s:%d;",sprintVCdbAtrributeFromToken(attributeNameTemp,apps[i]->attributeTokens[j]),apps[i]->attributeValues[j]); strcat(attributeStrA,strTemp);}
						printf("%d\t%s\t%d\t%d\t%d\t%s\t%s\r\n",apps[i]->basevid,partsList[apps[i]->partsListid],apps[i]->parttype,apps[i]->position,apps[i]->qty,attributeStrA,apps[i]->notes);
					}
				}
				break;
			case 2:
			// human-raedable
				if(vcdb_used)
				{
#ifdef WITH_MYSQL
					if(assetsFoundInXML)
					{// 1 or more asset tags were found in input xml file
						printf("make\tmodel\tyear\tpart\tparttypeid\tpositionid\tquantity\tqualifers\tasset\tasset_item_order\tnotes\r\n");
						for(i=0;i<=apps_count-1;i++)
						{
							sprintNiceQualifiers(attributeStrA, attributeStrB, apps[i]);
							printf("%s\t%s\t%s\t%s\t%d\t%s\t%s\t%d\t%s\t%s\r\n",sprintNiceMMY(niceMMYtemp,apps[i]->basevid, 9),partsList[apps[i]->partsListid],sprintNicePartTypeName(strPartTypeName,apps[i]->parttype),sprintNicePositionName(strPositionName,apps[i]->position),apps[i]->qty,attributeStrA,assetList[apps[i]->assetid],apps[i]->assetorder,apps[i]->notes,attributeStrB);
						}
					}
					else
					{// no asset tags were found in input xml - export without asset and asset-order columns
						printf("make\tmodel\tyear\tpart\tparttypeid\tpositionid\tquantity\tqualifers\tnotes\r\n");
						for(i=0;i<=apps_count-1;i++)
						{
							sprintNiceQualifiers(attributeStrA, attributeStrB, apps[i]);
							printf("%s\t%s\t%s\t%s\t%d\t%s\t%s\t%s\r\n",sprintNiceMMY(niceMMYtemp,apps[i]->basevid, 9),partsList[apps[i]->partsListid],sprintNicePartTypeName(strPartTypeName,apps[i]->parttype),sprintNicePositionName(strPositionName,apps[i]->position),apps[i]->qty,attributeStrA,apps[i]->notes,attributeStrB);
						}
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


	if(output_xml_filename[0])
	{
		fprintf(output_xml_fileptr,"<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>\r\n<ACES version=\"3.1\">");
		fprintf(output_xml_fileptr,"<Header></Header>");

/*
<Company>AutoPartSource</Company>
<SenderName>Luke Smith</SenderName>
<SenderPhone>800-777-5552</SenderPhone>
<TransferDate>2017-01-12</TransferDate>
<BrandAAIAID>BMNN</BrandAAIAID>
<DocumentTitle>AirQualitee</DocumentTitle>
<EffectiveDate>2017-01-12</EffectiveDate>
<SubmissionType>Full</SubmissionType>
<VcdbVersionDate>2016-12-30</VcdbVersionDate>
<QdbVersionDate>2014-02-25</QdbVersionDate>
<PcdbVersionDate>2014-02-28</PcdbVersionDate>

        <App action="A" id="2370">
                <BaseVehicle id="18242"/>
                <SubModel id="806"/>
                <Note>May not be standard equipment</Note>
                <Qty>1</Qty>
                <PartType id="6832"/>
                <Position id="1"/>
                <Part>AQ1040</Part>
        </App>


*/
		for(i=0;i<=apps_count-1;i++)
		{
			fprintf(output_xml_fileptr,"\t<App action=\"A\" id=\"%d\">\r\n\t\t<BaseVehicle id=\"%d\"/>\r\n",(i+1),apps[i]->basevid);
			for(j=0; j<=apps[i]->attributeCount-1; j++)
			{
				fprintf(output_xml_fileptr,"\t\t<%s id=\"%d\">\r\n",sprintVCdbAtrributeFromToken(attributeNameTemp,apps[i]->attributeTokens[j]),apps[i]->attributeValues[j]);
			}
			if(strlen(apps[i]->notes)){fprintf(output_xml_fileptr,"\t\t<Note>%s</Note>\r\n",apps[i]->notes);}
			fprintf(output_xml_fileptr,"\t\t<Qty>%d</Qty><PartType id=\"%d\"/><Position id=\"%d\"/><Part>%s</Part>\r\n\t</App>\r\n",apps[i]->qty,apps[i]->parttype,apps[i]->position,partsList[apps[i]->partsListid]);
		}

		fprintf(output_xml_fileptr,"\t<Footer>\r\n\t\t<RecordCount>%d</RecordCount>\r\n\t</Footer>\r\n</ACES>",apps_count);
		fclose(output_xml_fileptr);
	}




	xmlFreeDoc(doc);
	xmlCleanupParser();
	// free apps from memory
	for(i=0;i<=apps_count-1;i++)
	{
		free(apps[i]->notes);
		free(apps[i]);
	} apps_count=0;

	if(assessment_filename[0])
	{
		fprintf(assessment_fileptr,"</Workbook>");
		fclose(assessment_fileptr);
	}
}




char *sprintNicePartTypeName(char *target,int parttypeid)
{
	if(!pcdb_used){sprintf(target,"%d",parttypeid); return target;}
#ifdef WITH_MYSQL
	sprintf(sql_command,"select partterminologyname from parts where partterminologyid=%d;",parttypeid); if(mysql_query(&dbPCDB,sql_command)){printf("\nSQL Error\n%s\n",sql_command);exit(1);} dbPCDBRecset = mysql_store_result(&dbPCDB);
	dbPCDBRow = mysql_fetch_row(dbPCDBRecset);
	if(dbPCDBRow != NULL)
	{
		sprintf(target,"%s",dbPCDBRow[0]);
	}else{sprintf(target,"not found");}
	mysql_free_result(dbPCDBRecset);
#endif
	return target;
}

char *sprintNicePositionName(char *target,int positionid)
{
	if(!pcdb_used){sprintf(target,"%d",positionid); return target;}
#ifdef WITH_MYSQL
	sprintf(sql_command,"select positionname from position where positionid = %d;",positionid); if(mysql_query(&dbPCDB,sql_command)){printf("\nSQL Error\n%s\n",sql_command);exit(1);} dbPCDBRecset = mysql_store_result(&dbPCDB);
	dbPCDBRow = mysql_fetch_row(dbPCDBRecset);
	if(dbPCDBRow != NULL)
	{
		strcpy(target,dbPCDBRow[0]);
	}else{sprintf(target,"%d not found in PCdb",positionid);}
	mysql_free_result(dbPCDBRecset);
#endif
	return target;
}

char *sprintNiceMMY(char *target,int basevid, char delimiter)
{
	if(!vcdb_used){sprintf(target,"%d%c%d%c%d",basevid,delimiter,0,delimiter,0); return target;}
#ifdef WITH_MYSQL
	char makeName[255]="unknown";	char modelName[255]="unknown"; int year=0;

	sprintf(sql_command,"select makename,modelname,yearid from basevehicle,make,model where basevehicle.makeid = make.makeid and basevehicle.modelid = model.modelid and basevehicle.basevehicleid=%d;",basevid); if(mysql_query(&dbVCDB,sql_command)){printf("\nSQL Error\n%s\n",sql_command);exit(1);} dbVCDBRecset = mysql_store_result(&dbVCDB);
	dbVCDBRow = mysql_fetch_row(dbVCDBRecset);
	if(dbVCDBRow!=NULL)
	{
		sprintf(target,"%s%c%s%c%d",dbVCDBRow[0],delimiter,dbVCDBRow[1],delimiter,atoi(dbVCDBRow[2]));
	}
	else
	{
		sprintf(target,"?%c?%c?",delimiter,delimiter);
	}
	mysql_free_result(dbVCDBRecset);

#endif
	return target;
}

void fillNiceMMYstruct(struct MMY *targetMMY,int basevid)
{
	if(!vcdb_used){sprintf(targetMMY->makeName,"%d",basevid);targetMMY->modelName[0]=0; targetMMY->year=0; return;}
#ifdef WITH_MYSQL
	sprintf(sql_command,"select makename,modelname,yearid from basevehicle,make,model where basevehicle.makeid = make.makeid and basevehicle.modelid = model.modelid and basevehicle.basevehicleid=%d;",basevid); if(mysql_query(&dbVCDB,sql_command)){printf("\nSQL Error\n%s\n",sql_command);exit(1);} dbVCDBRecset = mysql_store_result(&dbVCDB);
	dbVCDBRow = mysql_fetch_row(dbVCDBRecset);
	if(dbVCDBRow!=NULL)
	{
		sprintf(targetMMY->makeName,"%s",dbVCDBRow[0]);
		sprintf(targetMMY->modelName,"%s",dbVCDBRow[1]);
		targetMMY->year=atoi(dbVCDBRow[2]);
	}
	else
	{
		sprintf(targetMMY->makeName,"not found");targetMMY->modelName[0]=0; targetMMY->year=0;
	}
	mysql_free_result(dbVCDBRecset);
#endif
}













#ifdef WITH_MYSQL



// build the "from" and "where" sql tables and join clauses for vcdb validation query based on the attributes in the reference app
// the purpose is to tease out a list of attribute names for knowing which tables to validate against. You could simply validate every app against a monolithic all-in-one 
// join of the entire vcdb - this is process-intensive (very slow). If we only include the tables in the join that the app referes-to, the query is faster and more memory effecient.
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
	char attributeNameTemp[256];

	for(i=0; i<=app->attributeCount-1; i++)
	{
		vcdbSystem=systemGroupOfAttribute(sprintVCdbAtrributeFromToken(attributeNameTemp,app->attributeTokens[i]));
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
		sprintAttributeWhere(strTemp, sprintVCdbAtrributeFromToken(attributeNameTemp,app->attributeTokens[i]), app->attributeValues[i]);
		strcat(whereClause,strTemp);
	}

	sprintf(strTemp,"vehicle.basevehicleid = %d ",app->basevid);
	strcat(whereClause,strTemp);
	fromClause[strlen(fromClause)-1]=0; //kill the trailing ','
}



char *sprintNiceQualifiers(char *target, char *errors, struct ACESapp *app)
{
	int j;
	char strTemp[1024];
	char strErrorsTemp[256]="";
	char attributeNameTemp[256];
	target[0]=0; // init the target string to null
	errors[0]=0;

	for(j=0; j<=app->attributeCount-1; j++)
	{
		strTemp[0]=0; strErrorsTemp[0]=0;
		sprintAttributeSQL(sql_command,sprintVCdbAtrributeFromToken(attributeNameTemp,app->attributeTokens[j]),app->attributeValues[j]);
		if(sql_command[0]!=0)
		{
			if(mysql_query(&dbVCDB,sql_command)){printf("\nSQL Error\n%s\n",sql_command);exit(1);}
			dbVCDBRecset = mysql_store_result(&dbVCDB);
			dbVCDBRow = mysql_fetch_row(dbVCDBRecset);
			if(dbVCDBRow==NULL)
			{// empty result set from query
				sprintf(strTemp,"invalid attribute (%s=%d); ",sprintVCdbAtrributeFromToken(attributeNameTemp,app->attributeTokens[j]),app->attributeValues[j]);
				strcpy(strErrorsTemp,strTemp);
			}
			else
			{// got result from query
				sprintf(strTemp,"%s; ",dbVCDBRow[0]);
				if(app->attributeTokens[j]==23){sprintf(strTemp,"Body code %s; ",dbVCDBRow[0]);}
				if(app->attributeTokens[j]==4){sprintf(strTemp,"%s Door; ",dbVCDBRow[0]);}
				if(app->attributeTokens[j]==10){sprintf(strTemp,"%s%s %sL; ",dbVCDBRow[4],dbVCDBRow[3],dbVCDBRow[0]);}
				if(app->attributeTokens[j]==13){sprintf(strTemp,"VIN:%s; ",dbVCDBRow[0]);}
				if(app->attributeTokens[j]==35){sprintf(strTemp,"%s Transmission; ",dbVCDBRow[0]);}
				if(app->attributeTokens[j]==32){sprintf(strTemp,"%s %s Speed %s; ",dbVCDBRow[0],dbVCDBRow[1],dbVCDBRow[2]);}
				if(app->attributeTokens[j]==33){sprintf(strTemp,"%s Transmission; ",dbVCDBRow[0]);}
				if(app->attributeTokens[j]==36){sprintf(strTemp,"%s Speed Transmission; ",dbVCDBRow[0]);}
				if(app->attributeTokens[j]==34){sprintf(strTemp,"%s Transmission; ",dbVCDBRow[0]);}
				if(app->attributeTokens[j]==2){sprintf(strTemp,"%s Inch Bed; ",dbVCDBRow[0]);}
				if(app->attributeTokens[j]==3){sprintf(strTemp,"%s Bed; ",dbVCDBRow[0]);}
				if(app->attributeTokens[j]==40){sprintf(strTemp,"%s Inch Wheelbase; ",dbVCDBRow[0]);}
				if(app->attributeTokens[j]==7){sprintf(strTemp,"%s Brakes; ",dbVCDBRow[0]);}
				if(app->attributeTokens[j]==15){sprintf(strTemp,"Front %s; ",dbVCDBRow[0]);}
				if(app->attributeTokens[j]==25){sprintf(strTemp,"Rear %s; ",dbVCDBRow[0]);}
				if(app->attributeTokens[j]==16){sprintf(strTemp,"Front %s Suspenssion; ",dbVCDBRow[0]);}
				if(app->attributeTokens[j]==26){sprintf(strTemp,"Rear %s Suspenssion; ",dbVCDBRow[0]);}
				if(app->attributeTokens[j]==28){sprintf(strTemp,"%s Steering; ",dbVCDBRow[0]);}
				if(app->attributeTokens[j]==29){sprintf(strTemp,"%s Steering; ",dbVCDBRow[0]);}
				if(app->attributeTokens[j]==38){sprintf(strTemp,"%s Valve; ",dbVCDBRow[0]);}
			}
			mysql_free_result(dbVCDBRecset);
		}
		else
		{// unknown attribute name - unlikely if this file passed xsd validation 
			sprintf(strTemp,"unknown vehicle attribute token (%s=%d); ",app->attributeTokens[j],app->attributeValues[j]);
			strcpy(strErrorsTemp,strTemp);
		}
		strcat(target,strTemp); strcat(errors,strErrorsTemp);
	}
	return target;
}





void sprintNiceAttribute(char *target, char *errors, struct ACESapp *app, int attributeIndex)
{
	char strTemp[1024];
	char strErrorsTemp[256]="";
	char attributeNameTemp[256];
	target[0]=0; // init the target string to null
	errors[0]=0;

	if(attributeIndex>(app->attributeCount-1)){sprintf(strErrorsTemp,"invalid attribute index"); return;}

	sprintAttributeSQL(sql_command,sprintVCdbAtrributeFromToken(attributeNameTemp,app->attributeTokens[attributeIndex]),app->attributeValues[attributeIndex]);
	if(sql_command[0]!=0)
	{
		if(mysql_query(&dbVCDB,sql_command)){printf("\nSQL Error\n%s\n",sql_command);exit(1);}
		dbVCDBRecset = mysql_store_result(&dbVCDB);
		dbVCDBRow = mysql_fetch_row(dbVCDBRecset);
		if(dbVCDBRow==NULL)
		{// empty result set from query
			sprintf(strTemp,"invalid attribute (%s = %d); ", sprintVCdbAtrributeFromToken(attributeNameTemp, app->attributeTokens[attributeIndex]) ,app->attributeValues[attributeIndex]);
			strcpy(strErrorsTemp,strTemp);
		}
		else
		{// got result from query
			sprintf(strTemp,"%s; ",dbVCDBRow[0]);
			if(app->attributeTokens[attributeIndex]==23){sprintf(strTemp,"Body code %s; ",dbVCDBRow[0]);}
			if(app->attributeTokens[attributeIndex]==4){sprintf(strTemp,"%s Door; ",dbVCDBRow[0]);}
			if(app->attributeTokens[attributeIndex]==10){sprintf(strTemp,"%s%s %sL; ",dbVCDBRow[4],dbVCDBRow[3],dbVCDBRow[0]);}
			if(app->attributeTokens[attributeIndex]==13){sprintf(strTemp,"VIN:%s; ",dbVCDBRow[0]);}
			if(app->attributeTokens[attributeIndex]==35){sprintf(strTemp,"%s Transmission; ",dbVCDBRow[0]);}
			if(app->attributeTokens[attributeIndex]==32){sprintf(strTemp,"%s %s Speed %s; ",dbVCDBRow[0],dbVCDBRow[1],dbVCDBRow[2]);}
			if(app->attributeTokens[attributeIndex]==33){sprintf(strTemp,"%s Transmission; ",dbVCDBRow[0]);}
			if(app->attributeTokens[attributeIndex]==36){sprintf(strTemp,"%s Speed Transmission; ",dbVCDBRow[0]);}
			if(app->attributeTokens[attributeIndex]==34){sprintf(strTemp,"%s Transmission; ",dbVCDBRow[0]);}
			if(app->attributeTokens[attributeIndex]==2){sprintf(strTemp,"%s Inch Bed; ",dbVCDBRow[0]);}
			if(app->attributeTokens[attributeIndex]==3){sprintf(strTemp,"%s Bed; ",dbVCDBRow[0]);}
			if(app->attributeTokens[attributeIndex]==40){sprintf(strTemp,"%s Inch Wheelbase; ",dbVCDBRow[0]);}
			if(app->attributeTokens[attributeIndex]==7){sprintf(strTemp,"%s Brakes; ",dbVCDBRow[0]);}
			if(app->attributeTokens[attributeIndex]==15){sprintf(strTemp,"Front %s; ",dbVCDBRow[0]);}
			if(app->attributeTokens[attributeIndex]==25){sprintf(strTemp,"Rear %s; ",dbVCDBRow[0]);}
			if(app->attributeTokens[attributeIndex]==16){sprintf(strTemp,"Front %s Suspenssion; ",dbVCDBRow[0]);}
			if(app->attributeTokens[attributeIndex]==26){sprintf(strTemp,"Rear %s Suspenssion; ",dbVCDBRow[0]);}
			if(app->attributeTokens[attributeIndex]==28){sprintf(strTemp,"%s Steering; ",dbVCDBRow[0]);}
			if(app->attributeTokens[attributeIndex]==29){sprintf(strTemp,"%s Steering; ",dbVCDBRow[0]);}
			if(app->attributeTokens[attributeIndex]==38){sprintf(strTemp,"%s Valve; ",dbVCDBRow[0]);}
		}
		mysql_free_result(dbVCDBRecset);
	}
	else
	{// unknown attribute name - unlikely if this file passed xsd validation 
		sprintf(strTemp,"unknown vehicle attribute token (%s=%d); ",app->attributeTokens[attributeIndex],app->attributeValues[attributeIndex]);
		strcpy(strErrorsTemp,strTemp);
	}
	strcat(target,strTemp); strcat(errors,strErrorsTemp);
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
	if(strcmp(name,"ValvesPerEngine")==0){sprintf(target,"engineconfig.valvesid = %d and ",value);return;}
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
// basevid/parttype/position/part/mfrlabel/qualifiers&notes/asset/assetorder
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
          if(ptra->partsListid > ptrb->partsListid)
          {// ptra->part > ptrb->part
           return(+1);
          }
          else
          {//ptra->part <= ptrb->part
           if(ptra->partsListid==ptrb->partsListid)
           {/// basebids are equal, parttypes are equal, positions are equal, parts are equal
            if(ptra->mfrlabelid > ptrb->mfrlabelid)
            {// mfrlabels A > B
             return(+1);
            }
            else
            {// mfrlabels A <= B
             if(ptra->mfrlabelid == ptrb->mfrlabelid)
             {
              // now contemplate attribute pairs and notes - this requires inflating them into strings (like: Submodel:103;EngineBase:33;Green Paint)
              attributeStrA[0]=0; attributeStrB[0]=0;
              for(i=0; i<=ptra->attributeCount-1; i++){snprintf(tempStr,1024,"%d:%d;",ptra->attributeTokens[i],ptra->attributeValues[i]); strcat(attributeStrA,tempStr);} strcat(attributeStrA,ptra->notes);
              for(i=0; i<=ptrb->attributeCount-1; i++){snprintf(tempStr,1024,"%d:%d;",ptrb->attributeTokens[i],ptrb->attributeValues[i]); strcat(attributeStrB,tempStr);} strcat(attributeStrB,ptrb->notes);
              strCmpResults=strcmp(attributeStrA,attributeStrB);

              if(strCmpResults>0)
              {// qualifiers A > qualifiers B
               return(+1);
              }
              else
              {//qualifiers A <= qualifiers B
               if(strCmpResults==0)
               {// basebids are equal, parttypes are equal, positions are equal, parts are equal, mfrlabels are equal, qualifiers are equal,
                if(ptra->assetid > ptrb->assetid)
                {//assetid A > assetid B
                 return(+1);
                }
                else
                {
                 if(ptra->assetid == ptrb->assetid)
                 {// basebids are equal, parttypes are equal, positions are equal, parts are equal, mfrlabels are equal, qualifiers are equal, assetid's are equal
                  if(ptra->assetorder > ptrb->assetorder)
                  {//assetorder A > assetorder B
                   return(+1);
                  }
                  else
                  {
                   if(ptra->assetorder == ptrb->assetorder)
                   {// basebids are equal, parttypes are equal, positions are equal, parts are equal, mfrlabels are equal, qualifiers are equal, assetid's are equal, asset orders are equal
                    return(0);
                   }
                   else
                   {//assetorder A < assetorder B
                    return(-1);
                   }
                  }
                 }
                 else
                 {//assetid A < assetid B
                  return(-1);
                 }
                }
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




char vcdbAtrributeToken(char *ref)
{
	if(strcmp(ref,"Aspiration")==0){return 1;}
	if(strcmp(ref,"BedLength")==0){return 2;}
	if(strcmp(ref,"BedType")==0){return 3;}
	if(strcmp(ref,"BodyNumDoors")==0){return 4;}
	if(strcmp(ref,"BodyType")==0){return 5;}
	if(strcmp(ref,"BrakeABS")==0){return 6;}
	if(strcmp(ref,"BrakeSystem")==0){return 7;}
	if(strcmp(ref,"CylinderHeadType")==0){return 8;}
	if(strcmp(ref,"DriveType")==0){return 9;}
	if(strcmp(ref,"EngineBase")==0){return 10;}
	if(strcmp(ref,"EngineDesignation")==0){return 11;}
	if(strcmp(ref,"EngineMfr")==0){return 12;}
	if(strcmp(ref,"EngineVIN")==0){return 13;}
	if(strcmp(ref,"EngineVersion")==0){return 14;}
	if(strcmp(ref,"FrontBrakeType")==0){return 15;}
	if(strcmp(ref,"FrontSpringType")==0){return 16;}
	if(strcmp(ref,"FuelDeliverySubType")==0){return 17;}
	if(strcmp(ref,"FuelDeliveryType")==0){return 18;}
	if(strcmp(ref,"FuelSystemControlType")==0){return 19;}
	if(strcmp(ref,"FuelSystemDesign")==0){return 20;}
	if(strcmp(ref,"FuelType")==0){return 21;}
	if(strcmp(ref,"IgnitionSystemType")==0){return 22;}
	if(strcmp(ref,"MfrBodyCode")==0){return 23;}
	if(strcmp(ref,"PowerOutput")==0){return 24;}
	if(strcmp(ref,"RearBrakeType")==0){return 25;}
	if(strcmp(ref,"RearSpringType")==0){return 26;}
	if(strcmp(ref,"Region")==0){return 27;}
	if(strcmp(ref,"SteeringSystem")==0){return 28;}
	if(strcmp(ref,"SteeringType")==0){return 29;}
	if(strcmp(ref,"SubModel")==0){return 30;}
	if(strcmp(ref,"TransElecControlled")==0){return 31;}
	if(strcmp(ref,"TransmissionBase")==0){return 32;}
	if(strcmp(ref,"TransmissionControlType")==0){return 33;}
	if(strcmp(ref,"TransmissionMfr")==0){return 34;}
	if(strcmp(ref,"TransmissionMfrCode")==0){return 35;}
	if(strcmp(ref,"TransmissionNumSpeeds")==0){return 36;}
	if(strcmp(ref,"TransmissionType")==0){return 37;}
	if(strcmp(ref,"ValvesPerEngine")==0){return 38;}
	if(strcmp(ref,"VehicleType")==0){return 39;}
	if(strcmp(ref,"WheelBase")==0){return 40;}
	return 0;
}




char *sprintVCdbAtrributeFromToken(char *target,char token)
{
	switch(token)
	{
	  case 0: sprintf(target,"unknown"); break;
	  case 1: sprintf(target,"Aspiration"); break;
	  case 2: sprintf(target,"BedLength"); break;
	  case 3: sprintf(target,"BedType"); break;
	  case 4: sprintf(target,"BodyNumDoors"); break;
	  case 5: sprintf(target,"BodyType"); break;
	  case 6: sprintf(target,"BrakeABS"); break;
	  case 7: sprintf(target,"BrakeSystem"); break;
	  case 8: sprintf(target,"CylinderHeadType"); break;
	  case 9: sprintf(target,"DriveType"); break;
	  case 10: sprintf(target,"EngineBase"); break;
	  case 11: sprintf(target,"EngineDesignation"); break;
	  case 12: sprintf(target,"EngineMfr"); break;
	  case 13: sprintf(target,"EngineVIN"); break;
	  case 14: sprintf(target,"EngineVersion"); break;
	  case 15: sprintf(target,"FrontBrakeType"); break;
	  case 16: sprintf(target,"FrontSpringType"); break;
	  case 17: sprintf(target,"FuelDeliverySubType"); break;
	  case 18: sprintf(target,"FuelDeliveryType"); break;
	  case 19: sprintf(target,"FuelSystemControlType"); break;
	  case 20: sprintf(target,"FuelSystemDesign"); break;
	  case 21: sprintf(target,"FuelType"); break;
	  case 22: sprintf(target,"IgnitionSystemType"); break;
	  case 23: sprintf(target,"MfrBodyCode"); break;
	  case 24: sprintf(target,"PowerOutput"); break;
	  case 25: sprintf(target,"RearBrakeType"); break;
	  case 26: sprintf(target,"RearSpringType"); break;
	  case 27: sprintf(target,"Region"); break;
	  case 28: sprintf(target,"SteeringSystem"); break;
	  case 29: sprintf(target,"SteeringType"); break;
	  case 30: sprintf(target,"SubModel"); break;
	  case 31: sprintf(target,"TransElecControlled"); break;
	  case 32: sprintf(target,"TransmissionBase"); break;
	  case 33: sprintf(target,"TransmissionControlType"); break;
	  case 34: sprintf(target,"TransmissionMfr"); break;
	  case 35: sprintf(target,"TransmissionMfrCode"); break;
	  case 36: sprintf(target,"TransmissionNumSpeeds"); break;
	  case 37: sprintf(target,"TransmissionType"); break;
	  case 38: sprintf(target,"ValvesPerEngine"); break;
	  case 39: sprintf(target,"VehicleType"); break;
	  case 40: sprintf(target,"WheelBase"); break;
	  default: sprintf(target,"UNKNOWN"); break;
	}
	return target;
}

