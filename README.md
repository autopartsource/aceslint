ACESlint is a simple command-line "Lint" tool for evaluating ACES xml (Automotive Catalog Exchange Standard) files.
ACES is a product of autocare.org (http://autocare.org/technology/). It is a way for automotive industry trading partners to 
share catalog (part-vehicle "fitment") in an organized way.

##Features  
* Check of duplicates and overlaps of applications
* Check of validity of base vehicle id's (VCdb data source required)
* Check of validity of attribute id's (VCdb data source required)
* Check of validity of combinaions of attribute id's against base vehicle (VCdb data source required)
* Check of validity of combinaions of parttype and position (PCdb data source required)
* Filter by parttype id - inclusive or exclusive
* Filter and translate Part tag in applications by two-column, tab-delimited interchange text file
* Filter by model-year range (VCdb data source required)
* Filter by vehicle makeID list - inclusive or exclusive (VCdb data source required) 
* Extract distinct Part list
* Extract distinct parttype id list
* Extract distinct AssetName list
* Flatten applications down to a tab-delimited text file of coded values
* Flatten applications down to a tab-delimited text file of human-readable values (VCdb data source required)


##Overview
 The ACES spec only defines a basic structure for communicating "application" data between trading partners. AutoCare.org
provides an XSD schema file for validating the structural correctness of the ACES xml file. The spec does not clearly define
rules about the quality of the data conveyed. For example, a common quality problem is an "overlap".
This is where two different "App" nodes in the xml refer to the same basevehicle with different partnumbers. The real-world
consequence of this is that the end consumer is faced with two different parts for his/her car and no clear understanding 
of which is the right choice. There are many other possible quality problems that could be found in a valid ACES file. It is
generally up to the the data receiver to assess the acceptability of an ACES file and give feedback to the supplier. Many 
data suppliers (part manufacturers) employ sophisticated catalog management systems to house and export ACES files for their
trading partners. These systems generally include quality assurance checks on the outbound data. ACESlint is intended to be
fast and simple "second opinion" of an outbound file, or a quick cleanliness checker for an inbound file before spending time
to import a file of unknown quality. On a typical workstation-grade PC, ACESlint will process about 100,000 applications
per minute, and consume about 400MB per 100,000 applications.

 ACESlint's source is completely contained in a ".c" file. It is intended to be compiled and run on a Linux-ish system. 
It requires libxml2 to be present on the local system. If MySQL support is compiled-in, you can specify a databases(es) to validate 
VCdb and/or PCdb codes against. Obvioulsy, those databases are expected to have a very specific structure (see below). If an ACES xml file is 
linted without referencing a database, only partial validation will be performed (duplicates, overlaps and CNC overlaps) and human-readable 
"flattened" ouptut is disabled. If a VCdb database is provided, extended validation is done on the file (basevehicle id validation, attribute code validation 
and attribute combinations validation). Providing a database will also allow the tool to translate VCdb attribute codes and 
basevehicle id's into human-readable values for easier consumption of the results.

If you are interested in using/testing/contributing, feel free to contact Luke Smith lsmith@autopartsource.com. Our industry
can benefit from open-source tools and collaboration. We want to connect with other industry people who share this view.
If this tool looks like it would be helpful but the whole linux thing is a bit overwhelming, we will guide you through the 
process of getting up and running.


The input xml file is processed in this order:

* Import apps from xml file
* (optionally) Filter by items, modelyears, makes parttypes etc.
* Validate against specified VCdb for basevehicleid's, attribute id's and combinations for attributes
* Search for duplicates, overlaps and comment-no-comment (CNC) errors
* Validate against specified PCdb for parttype-position combinations
* (optionally) Print out distinct items list
* (optionally) Print out distinct parttypeid list
* (optionally) Print out distinct assets list
* (optionally) Print out "flattened" applications

Please note, only basevehicleid-oriented datasets are supported. In other words, apps must identify their vehicle like:  

``        <App action="A" id="1">``  
``                <BaseVehicle id="5888"/>``  
``                <Qty>1</Qty>``  
``                <PartType id="6832"/>``  
``                <Position id="1"/>``  
``                <Part>AQ1007</Part>``  
``        </App>``  



#Compilation
You will have to have libxml2 installed. On a modern Fedora system:  
``dnf install libxml2-devel``  


##with mysql support
You will need the mysql client source files.
``dnf install mysql-devel``  


After you install the mysql-devel package, you should have the client lib files (including ``libmysqlclient.so``) present on your system - probably in /usr/lib/mysql  
If they were installed somewhere else, that will need to be reflected in the ``-L/usr/lib/mysql`` section of the compile command:  


``gcc -o aceslint `xml2-config --cflags` aceslint.c `xml2-config --libs` -L/usr/lib/mysql -lmysqlclient -lz -DWITH_MYSQL``


##without mysql support - PCdb and VCdb features will be disabled
``gcc -o aceslint `xml2-config --cflags` aceslint.c `xml2-config --libs` ``  


#Running

At minimum, a single argument of input xml filename is required:  

``aceslint ACESfilename.xml [options]``

### Options are expressed with command-line switches:
* -vcdb &lt;VCdb database name&gt; (example vcdb20170127)
* -pcdb &lt;PCdb database name&gt; (example pcdb20170210)
* -h &lt;database host&gt; (optional - "localhost" is assumed)
* -u &lt;database user&gt; (optional - "" is assumed)
* -p &lt;database password&gt; (optional - "" is assumed)
* -v &lt;verbosity level&gt; (optional - 1 is assumed)
* --ignorenaparts (ignore apps with "NA" as the part number)
* --parttranslationfile &lt;filename&gt; (translate part number by a 2-column, tab-delimited interchange. Apps with no interchange will be dropped)
* --filterbyyears &lt;from year&gt; &lt;to year&gt; (discard all apps outside given range. ex: "--filterbyyears 2010 2012" only preserves 2010,2011, 2012)
* --includeparttypeids  &lt;parttypeid1,parttypeid2,parttypeid3...&gt; (discard all apps outside of given parttypeids. ex "--includeparttypeids 6832" only preserves Cabin Air Filters)
* --excludeparttypeids  &lt;parttypeid1,parttypeid2,parttypeid3...&gt; (discard all apps in given parttypeids. ex "--excludeparttypeids 6832" discards Cabin Air Filters)
* --includemakeids  &lt;makeid1,makeid2,makeid3...&gt; (discard all apps outside of given makeID's. ex "--includemakeids 75,76" only preserves Lexus and Toyota apps)
* --excludemakeids  &lt;makeid1,makeid2,makeid3...&gt; (discard all apps in given makeID's. ex "--excludemakeids 75,76" discards Lexus and Toyota apps)
* --extractparts (surpress all other output and dump distinct list of part numbers found in the input file)
* --extractparttypes (surpress all other output and dump distinct list of part types found in the input file)
* --extractassets (surpress all other output and dump distinct list of assets names found in the input file)
* --flattenmethod &lt;method number&gt; (export a "flat" list of applications as tab-delimited data. Method 1 is VCdb-coded values, Method 2 is human-readable)


##example 1 (simple highlevel database-less audit)

``aceslint ACES_3_1_AirQualitee_FULL_2017-01-12.xml``

### will produce output like:
``Title:AirQualitee``  
``VcdbVersionDate:2016-12-30``  
``Application count:6512``  
``Duplicate apps:0``  
``Overlaps:211``  
``CNC overlaps:530``  


##example 2 (verbose database-less audit saved to audit.txt)

``aceslint ACES_3_1_AirQualitee_FULL_2017-01-12.xml -v 3 > audit.txt``  

The specific errors detected will be saved to the text file in a human-readable format. The data is tab-delimited 
for easy importation to a spreadsheet for deeper inspection.


##example 3 (referencing a specific VCdb database for code validation)

``aceslint ACES_3_1_AirQualitee_FULL_2017-01-12.xml -vcdb vcdb20171231``  

### will produce output like:
``Title:AirQualitee``  
``VcdbVersionDate:2016-12-30``  
``Application count:6512``  
``Invalid basevids:0``  
``Invalid vcdb codes:0``  
``Invalid vcdb configurations:0``  
``Duplicate apps:0``  
``Overlaps:211``  
``CNC overlaps:529``  


##example 4 (extracting a distinct list of parts)  

``aceslint ACES_3_1_AirQualitee_FULL_2017-01-12.xml --extractparts`` 

### will produce output like (only fist 10 of 257 lines shown): 
``AQ1056`` 
``AQ1124`` 
``AQ1165`` 
``AQ1136C`` 
``AQ1022`` 
``AQ1021`` 
``AQ1053`` 
``AQ1069`` 
``AQ1190`` 
``AQ1092`` 


##example 5 (extracting parts applied to Lexus and Toyota vehicles)

``aceslint ACES_3_1_AirQualitee_FULL_2017-01-12.xml -vcdb vcdb20170127 --extractparts --includemakeids 75,76``  

### will produce output like:  
``AQ1060``  
``AQ1062``  
``AQ1048``  
``AQ1048C``  
``AQ1044``  
``AQ1044C``  
``AQ1072``  
``AQ1102``  
``AQ1102C``  
``AQ1045``  
``AQ1117``  


##example 6 (extracting parts applied to Lexus and Toyota vehicles in modelyears 2014-2017)

``aceslint ACES_3_1_AirQualitee_FULL_2017-01-12.xml -vcdb vcdb20170127 --extractparts --includemakeids 75,76 --filterbyyears 2014 2017``  

### will produce output like:  
``AQ1102``  
``AQ1102C``  
``AQ1060``  
``AQ1045``  


##example 7 (extracting parts applied to Lexus and Toyota vehicles in modelyears 2014-2017 that are parttypeid 6832)

``aceslint ACES_3_1_AirQualitee_FULL_2017-01-12.xml -vcdb vcdb20170127 --extractparts --includemakeids 75,76 --filterbyyears 2014 2017 --includeparttypeids 6832``  

### will produce output like:  
``AQ1102``  
``AQ1102C``  
``AQ1060``  
``AQ1045``  



##example 8 (extracting parts in modelyears 2001-2010 that are parttypeid 11292)

``aceslint ACES_3_1_AirQualitee_FULL_2017-01-12.xml -vcdb vcdb20170127 --extractparts --filterbyyears 2001 2010 --includeparttypeids 11292``  

### will produce output like:  
``AQH011``  
``AQH004``  
``AQH004R``  
``AQH005``  
``AQH005R``  
``AQH012``  
``AQH006``  
``AQH006R``  
``AQH008``  
``AQH008R``  
``AQH007R``  
``AQH001``  
``AQH001R``  
``AQH002``  
``AQH002R``  
``AQH003``  
``AQH003R``  




##example 9 (extracting a distinct list of part types)

``aceslint ACES_3_1_AirQualitee_FULL_2017-01-12.xml --extractparttypes`` 

### will produce output like:
``6832``  
``11292``  
``12819``  
``14335``  

##example 10 (extracting a distinct list of part types with nice names - requires PCdb)

``aceslint ACES_3_1_AirQualitee_FULL_2017-01-12.xml -pcdb pcdb20170210 --extractparttypes`` 

### will produce output like:
``6832	Cabin Air Filter``  
``11292	Cabin Air Filter Retainer``  
``12819	Cabin Air Filter Case``  
``14335	Cabin Air Filter Screw``  


##example 11 (export to textfile a "flattened" (spreadsheet) version of the input file - coded VCdb values)

``aceslint ACES_3_1_AirQualitee_FULL_2017-01-12.xml --flattenmethod 1 > myflatfile.txt`` 

### myflatfile.txt will contain (only a few line shown):
``basevid part    parttypeid      positionid      quantity        qualifers       notes``  
``138     AQ1136C 6832    1       1``  
``169     AQ1022  6832    1       1               vehicle may not be equipped with cabin filter but vehicle has housing and filter can be added;``  
``171     AQ1022  6832    1       1       SubModel:92;``  
``172     AQ1022  6832    1       1       SubModel:92;``  
``174     AQ1022  6832    1       1       SubModel:92;``  
``177     AQ1022  6832    1       1       SubModel:113;``  
``177     AQ1022  6832    1       1       SubModel:92;``  
``178     AQ1021  6832    1       1``  
``179     AQ1022  6832    1       1               may not be standard equipment;``  


##example 12 (export to textfile a "flattened" (spreadsheet) version of the input file - human-readable values - requires VCdb)

``aceslint ACES_3_1_AirQualitee_FULL_2017-01-12.xml -vcdb vcdb20170127 --flattenmethod 1 > myflatfile.txt`` 

### myflatfile.txt will contain (only a few line shown):
``Porsche Cayenne 2003    6832    1       1       AQ1136C``  
``Hyundai Santa Fe        2001    6832    1       1       AQ1022          vehicle may not be equipped with cabin filter but vehicle has housing and filter can be added;``  
``Hyundai Sonata  1999    6832    1       1       AQ1022  GLS;``  
``Hyundai Sonata  2000    6832    1       1       AQ1022  GLS;``  
``Hyundai Sonata  2001    6832    1       1       AQ1022  GLS;``  
``Hyundai Sonata  2002    6832    1       1       AQ1022  LX;``  
``Hyundai Sonata  2002    6832    1       1       AQ1022  GLS;``  
``Hyundai Accent  2000    6832    1       1       AQ1021``  
``Hyundai XG300   2001    6832    1       1       AQ1022          may not be standard equipment;``  
``Hyundai Santa Fe        2002    6832    1       1       AQ1022          vehicle may not be equipped with cabin filter but vehicle has housing and filter can be added;``  
``Hyundai Sonata  2003    6832    1       1       AQ1022  LX;``  
``Hyundai Sonata  2003    6832    1       1       AQ1022  GLS;``  






#Database Creation
The VCdb ("Vehicle Configuration database") is published by AutoCare.org to subscribers on a monthly basis. The database contains 73 tables. Several 
different file formats are provided by AutoCare. We (AutoPartSource) use the "ASCII Text" version. It consists of 73 individual tab-delimited text files
that are named like: "20170127_BaseVehicle.txt". These files are delivered in a zip container. The shell script "rename_VCDB_import_files.sh" will 
rename all the text files to the generic table names that the database inporter script "load_VCDB_tables.sql" expects. VCDB_schema.sql contains the 
table creation SQL statements for making an empty VCdb on a MySQL server. Ideally, you would create a new, unique-named database to hold each new monthly 
release from Autocare.
