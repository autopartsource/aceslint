#aceslint
ACESlint is a simple command-line "Lint" tool for evaluating ACES xml (Automotive Catalog Enhanced Standard) files.
ACES is a product of autocare.org (http://autocare.org/technology/). It is a way for automotive industry trading partners to 
share catalog (part-vehicle "fitment") in an organized way.

 The ACES spec only defines a basic structure for communicating "application" data between trading partners. AutoCare.org
provides an XSD schema file for validating the structural correctness of the ACES xml file. The spec does not clearly define
rules about the quality of the data conveyed. For example, a common quality problem is an "overlap".
This is where two different "App" nodes in the xml refer to the same basevehicle with different partnumbers. The real-world
consequence of this is that the end consumer is faced with two different parts for his/her car and no clear understanding 
of which is the right choice. There are many other posible quality problems that could be found in a valid ACES file. It is
generally up to the the data receiver to assess the acceptibility of an ACES file and give feedback to the supplier. Many 
data suppliers (part manufacturers) employ sophisticated catalog management systems to house and export ACES files for their
trading partners. These systems generally include quality assurance checks on the outbound data. ACESlint is intended to be
fast and simple "second opinion" of an outbound file, or a quick cleanliness checker for an inboud file before spending time
to import a file of unknown quality. On a typical workstation-grade PC, a 100,000 application file can be analyzed in about 
30 seconds.

 ACESlint's source is completely contained in a ".c" file. It is intended to compiled and run on a Linux-ish system. 
It requires libxml2 to be present on the local system. If MySQL support is compiled-in, you can specify a database to validate 
VCdb codes against. If an ACES xml file is linted without referencing a database, only partial validation will be performed 
(duplicates, overlaps and CNC overlaps). If a VCdb database is provided, extended validation is done on the file 
(basevehicle id validation, attribute code validation  and attribute combinations validation)

If you are interested in using/testing/contributing, feel free to contact Luke Smith lsmith@autopartsource.com. Our industry
can benefit from open-source tools and collaboration. 


Compilation
----------------------------

With mysql support
------------------
gcc -o aceslint `xml2-config --cflags` aceslint.c `xml2-config --libs` -L/usr/lib/mysql -lmysqlclient -lz -DWITH_MYSQL


Without mysql support (VCdb features will be disabled)
------------------
gcc -o aceslint `xml2-config --cflags` aceslint.c `xml2-config --libs` -L/usr/lib/mysql -lmysqlclient -lz



Running
---------------------------

command-line switches:
-----------------
-d  database name (example vcdb20161231)
-h  database host ("localhost" is assumed if switch is not used)
-u  database user ("" is assumed if switch is not used. This is valid for a typical MySQL installation where no permissions or users have been defined)
-p  database password ("" is assumed if switch is not used. This is valid for a typical MySQL installation where no permissions or users have been defined)
-v  verbosity level (0 is assumed if switch is not used. Level 0 will give only a high-level listing or audit results)


example 1 - simple database-less audit

aceslint ACES_3_1_AirQualitee_FULL_2017-01-12.xml

example 2 - referencing a specific database for code validation

aceslint ACES_3_1_AirQualitee_FULL_2017-01-12.xml -d vcdb20171231



database creation
--------------------------
VCDB_schema.sql contains the table creation SQL statements for making an empty VCdb on a MySQL server. 
load_VCDB_tables.sql contains the script that imports tab-delimited text data files into database structure. AutoCare.org offer 
"ascii text" as one of the download options to its subscribers.









