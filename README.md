ACESlint is a simple command-line "Lint" tool for evaluating ACES xml (Automotive Catalog Enhanced Standard) files.
ACES is a product of autocare.org (http://autocare.org/technology/). It is a way for automotive industry trading partners to 
share catalog (part-vehicle "fitment") in an organized way.

##Features  
* Check of duplicates and overlaps of applications
* Check of validity of base vehicle id's (VCdb data source required)
* Check of validity of attribute id's (VCdb data source required)
* Check of validity of combinaions of attribute id's against base vehicle (VCdb data source required)
* Filter by model-year range (VCdb data source required)
* Filter by vehicle makeID list - inclusive or exclusive (VCdb data source required) 
* Extract distinct Part list
* Extract distinct AssetName list
* Interchange and filter applications by file of item=item
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
It requires libxml2 to be present on the local system. If MySQL support is compiled-in, you can specify a database to validate 
VCdb codes against. Obvioulsy, that database is expected to have a very specific structure (see below). If an ACES xml file is 
linted without referencing a database, only partial validation will be performed (duplicates, overlaps and CNC overlaps). 
If a VCdb database is provided, extended validation is done on the file (basevehicle id validation, attribute code validation 
and attribute combinations validation). Providing a database will also allow the tool to translate VCdb attribute codes and 
basevehicle id's into human-readable values for easier consumption of the results.

If you are interested in using/testing/contributing, feel free to contact Luke Smith lsmith@autopartsource.com. Our industry
can benefit from open-source tools and collaboration. 


The input xml file is processed in this order:

* Import apps from xml file
* (optionally) Filter by items, modelyears, makes parttypes etc.
* Search for duplicates
* Search for overlaps
* Search for comment-no-comment (CNC) errors
* Validate against VCdb for basevehicleid's, attribute id's and combinations for attributes
* (optionally) Print out distinct items list
* (optionally) Print out distinct assets list






#Compilation
##with mysql support
``gcc -o aceslint `xml2-config --cflags` aceslint.c `xml2-config --libs` -L/usr/lib/mysql -lmysqlclient -lz -DWITH_MYSQL``


##without mysql support - VCdb features will be disabled
``gcc -o aceslint `xml2-config --cflags` aceslint.c `xml2-config --libs` -L/usr/lib/mysql -lmysqlclient -lz``


#Running

At minimum, a single argument of input xml filename is required:  
``./aceslint &lt;ACESfilename.xml&gt; [options]


### Options are expressed with command-line switches:
* -d &lt;database name&gt; (example vcdb20161231)
* -h &lt;database host&gt; (optional - "localhost" is assumed)
* -u &lt;database user&gt; (optional - "" is assumed)
* -p &lt;database password&gt; (optional - "" is assumed)
* -v &lt;verbosity level&gt; (optional - 1 is assumed)
* --ignorenaitems (ignore apps with "NA" as the part number)
* --filterbyyears &lt;from year&gt; &lt;to year&gt; (discard all apps outside given range. ex: "--filterbyyears 2010 2012" only preserves 2010,2011, 2012)
* --includemakeids  &lt;makeid1,makeid2,makeid3...&gt; (discard all apps outside of given makeID's. ex "--includemakeids 75,76 only preserves Lexus and Toyota apps)
* --excludemakeids  &lt;makeid1,makeid2,makeid3...&gt; (discard all apps in given makeID's. ex "--excludemakeids 75,76 discards Lexus and Toyota apps)
* --extractitems (surpress all other output and dump distinct list of part number found in the input file)
* --extractassets (surpress all other output and dump distinct list of assets names found in the input file)
* --flattenmethod &lt;method number&gt; (export a "flat" list of applications as tab-delimited data. Method 1 is VCdb-coded values, Method 2 is human-readable)
* --itemtranslationfile &lt;filename&gt; (translate part number by a 2-column, tab-delimited interchange. Apps with no interchange will be dropped)


##example 1 (simple highlevel database-less audit)

``aceslint ACES_3_1_AirQualitee_FULL_2017-01-12.xml``

### will produce output like:
``Title:AirQualitee``<br/>
``VcdbVersionDate:2016-12-30``<br/>
``Application count:6512``<br/>
``Duplicate apps:0``<br/>
``Overlaps:211``<br/>
``CNC overlaps:530``<br/>


##example 2 (verbose database-less audit saved to audit.txt)

``aceslint ACES_3_1_AirQualitee_FULL_2017-01-12.xml -v 3 > audit.txt``

The specific errors detected will be saved to the text file in a human-readable format. The data is tab-delimited 
for easy importation to a spreadsheet for deeper inspection.


##example 3 (referencing a specific database for code validation)

aceslint ACES_3_1_AirQualitee_FULL_2017-01-12.xml -d vcdb20171231

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

##example 4 (extracting items applied to a Lexus and Toyota vehicles)

aceslint ACES_3_1_AirQualitee_FULL_2017-01-12.xml -d vcdb20170127 --extractitems --includemakeids 75,76

### will produce output like:
``Title:AirQualitee``<br/>
``VcdbVersionDate:2016-12-30``<br/>
Title:AirQualitee
VcdbVersionDate:2016-12-30
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



#Database Creation
The VCdb ("Vehicle Configuration database") is published by AutoCare.org to subscribers on a monthly basis. The database contains 73 tables. Several 
different file formats are provided by AutoCare. We (AutoPartSource) use the "ASCII Text" version. It consists of 73 individual tab-delimited text files
that are named like: "20170127_BaseVehicle.txt". These files are delivered in a zip container. The shell script "rename_VCDB_import_files.sh" will 
rename all the text files to the generic table names that the database inporter script "load_VCDB_tables.sql" expects. VCDB_schema.sql contains the 
table creation SQL statements for making an empty VCdb on a MySQL server. 










