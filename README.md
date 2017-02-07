# aceslint
Simple command-line "Lint" tool for evaluating ACES xml (AAIA automotive catalog standard) files
The ACES standard is a product of autocare.org (http://autocare.org/technology/).
It is a way for automotive industry trading partners to share catalog (part-vehicle "fitment") in an organized way.

This tool is completely contained in a single .c source file. It requires libxml2 to be presenent on the local system.
It can be compiled with MySQL support ("-DWITH_MYSQL" gcc command-line switch). If MySQL support is compiled-in, you 
can specify a database to validate VCdb codes against. If an ACES xml file is linted without referencing a database,
only partial validation will be performed.

