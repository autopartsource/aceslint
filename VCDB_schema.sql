-- MySQL dump 10.16  Distrib 10.1.18-MariaDB, for Linux (x86_64)
--
-- Host: localhost    Database: aces
-- ------------------------------------------------------
-- Server version	10.1.18-MariaDB

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40014 SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;

--
-- Table structure for table `abbreviation`
--

DROP TABLE IF EXISTS `abbreviation`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `abbreviation` (
  `abbreviation` char(3) NOT NULL DEFAULT '',
  `description` varchar(20) NOT NULL DEFAULT '',
  `longdescription` varchar(200) DEFAULT '',
  PRIMARY KEY (`abbreviation`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `aspiration`
--

DROP TABLE IF EXISTS `aspiration`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `aspiration` (
  `aspirationid` int(11) NOT NULL AUTO_INCREMENT,
  `aspirationname` varchar(30) NOT NULL DEFAULT '',
  PRIMARY KEY (`aspirationid`)
) ENGINE=MyISAM AUTO_INCREMENT=17 DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `basevehicle`
--

DROP TABLE IF EXISTS `basevehicle`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `basevehicle` (
  `basevehicleid` int(11) NOT NULL AUTO_INCREMENT,
  `yearid` int(11) NOT NULL DEFAULT '0',
  `makeid` int(11) NOT NULL DEFAULT '0',
  `modelid` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`basevehicleid`),
  KEY `makeid` (`makeid`),
  KEY `modelid` (`modelid`),
  KEY `yearid` (`yearid`)
) ENGINE=MyISAM AUTO_INCREMENT=1000375 DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `bedconfig`
--

DROP TABLE IF EXISTS `bedconfig`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `bedconfig` (
  `bedconfigid` int(11) NOT NULL AUTO_INCREMENT,
  `bedlengthid` int(11) NOT NULL DEFAULT '0',
  `bedtypeid` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`bedconfigid`)
) ENGINE=MyISAM AUTO_INCREMENT=253 DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `bedlength`
--

DROP TABLE IF EXISTS `bedlength`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `bedlength` (
  `bedlengthid` int(11) NOT NULL AUTO_INCREMENT,
  `bedlength` char(10) NOT NULL DEFAULT '',
  `bedlengthmetric` char(10) NOT NULL DEFAULT '',
  PRIMARY KEY (`bedlengthid`)
) ENGINE=MyISAM AUTO_INCREMENT=183 DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `bedtype`
--

DROP TABLE IF EXISTS `bedtype`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `bedtype` (
  `bedtypeid` int(11) NOT NULL AUTO_INCREMENT,
  `bedtypename` varchar(50) NOT NULL DEFAULT '',
  PRIMARY KEY (`bedtypeid`)
) ENGINE=MyISAM AUTO_INCREMENT=17 DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `bodynumdoors`
--

DROP TABLE IF EXISTS `bodynumdoors`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `bodynumdoors` (
  `bodynumdoorsid` int(11) NOT NULL AUTO_INCREMENT,
  `bodynumdoors` char(3) NOT NULL DEFAULT '',
  PRIMARY KEY (`bodynumdoorsid`)
) ENGINE=MyISAM AUTO_INCREMENT=14 DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `bodystyleconfig`
--

DROP TABLE IF EXISTS `bodystyleconfig`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `bodystyleconfig` (
  `bodystyleconfigid` int(11) NOT NULL AUTO_INCREMENT,
  `bodynumdoorsid` int(11) NOT NULL DEFAULT '0',
  `bodytypeid` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`bodystyleconfigid`)
) ENGINE=MyISAM AUTO_INCREMENT=226 DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `bodytype`
--

DROP TABLE IF EXISTS `bodytype`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `bodytype` (
  `bodytypeid` int(11) NOT NULL AUTO_INCREMENT,
  `bodytypename` varchar(50) NOT NULL DEFAULT '',
  PRIMARY KEY (`bodytypeid`)
) ENGINE=MyISAM AUTO_INCREMENT=117 DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `brakeabs`
--

DROP TABLE IF EXISTS `brakeabs`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `brakeabs` (
  `brakeabsid` int(11) NOT NULL AUTO_INCREMENT,
  `brakeabsname` varchar(30) NOT NULL DEFAULT '',
  PRIMARY KEY (`brakeabsid`)
) ENGINE=MyISAM AUTO_INCREMENT=10 DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `brakeconfig`
--

DROP TABLE IF EXISTS `brakeconfig`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `brakeconfig` (
  `brakeconfigid` int(11) NOT NULL AUTO_INCREMENT,
  `frontbraketypeid` int(11) DEFAULT NULL,
  `rearbraketypeid` int(11) DEFAULT NULL,
  `brakesystemid` int(11) NOT NULL DEFAULT '0',
  `brakeabsid` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`brakeconfigid`)
) ENGINE=MyISAM AUTO_INCREMENT=55 DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `brakesystem`
--

DROP TABLE IF EXISTS `brakesystem`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `brakesystem` (
  `brakesystemid` int(11) NOT NULL AUTO_INCREMENT,
  `brakesystemname` varchar(30) NOT NULL DEFAULT '',
  PRIMARY KEY (`brakesystemid`)
) ENGINE=MyISAM AUTO_INCREMENT=13 DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `braketype`
--

DROP TABLE IF EXISTS `braketype`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `braketype` (
  `braketypeid` int(11) NOT NULL AUTO_INCREMENT,
  `braketypename` varchar(30) NOT NULL DEFAULT '',
  PRIMARY KEY (`braketypeid`)
) ENGINE=MyISAM AUTO_INCREMENT=7 DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `changeattributestates`
--

DROP TABLE IF EXISTS `changeattributestates`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `changeattributestates` (
  `ChangeAttributeStateID` int(3) unsigned NOT NULL,
  `ChangeAttributeState` varchar(255) NOT NULL,
  PRIMARY KEY (`ChangeAttributeStateID`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `changedetails`
--

DROP TABLE IF EXISTS `changedetails`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `changedetails` (
  `ChangeDetailID` int(10) unsigned NOT NULL,
  `ChangeID` int(10) unsigned DEFAULT NULL,
  `ChangeAttributeStateID` int(10) unsigned DEFAULT NULL,
  `TableNameID` int(10) unsigned DEFAULT NULL,
  `PrimaryKeyColumnName` varchar(255) DEFAULT NULL,
  `PrimaryKeyBefore` varchar(255) DEFAULT NULL,
  `PrimaryKeyAfter` varchar(255) DEFAULT NULL,
  `ColumnName` varchar(255) DEFAULT NULL,
  `ColumnValueBefore` varchar(255) DEFAULT NULL,
  `ColumnValueAfter` varchar(255) DEFAULT NULL,
  PRIMARY KEY (`ChangeDetailID`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `changereasons`
--

DROP TABLE IF EXISTS `changereasons`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `changereasons` (
  `ChangeReasonID` int(3) unsigned NOT NULL,
  `ChangeReason` varchar(255) NOT NULL,
  PRIMARY KEY (`ChangeReasonID`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `changes`
--

DROP TABLE IF EXISTS `changes`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `changes` (
  `ChangeID` int(10) unsigned NOT NULL,
  `RequestID` int(10) unsigned DEFAULT NULL,
  `ChangeReasonID` int(3) unsigned DEFAULT NULL,
  `RevDate` datetime DEFAULT NULL,
  PRIMARY KEY (`ChangeID`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `changetablenames`
--

DROP TABLE IF EXISTS `changetablenames`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `changetablenames` (
  `TableNameID` int(10) unsigned NOT NULL,
  `TableName` varchar(255) NOT NULL,
  `TableDescription` varchar(255) NOT NULL,
  PRIMARY KEY (`TableNameID`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `cylinderheadtype`
--

DROP TABLE IF EXISTS `cylinderheadtype`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `cylinderheadtype` (
  `cylinderheadtypeid` int(11) NOT NULL AUTO_INCREMENT,
  `cylinderheadtypename` varchar(30) NOT NULL DEFAULT '',
  PRIMARY KEY (`cylinderheadtypeid`)
) ENGINE=MyISAM AUTO_INCREMENT=15 DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `drivetype`
--

DROP TABLE IF EXISTS `drivetype`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `drivetype` (
  `drivetypeid` int(11) NOT NULL AUTO_INCREMENT,
  `drivetypename` varchar(30) NOT NULL DEFAULT '',
  PRIMARY KEY (`drivetypeid`)
) ENGINE=MyISAM AUTO_INCREMENT=27 DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `enginebase`
--

DROP TABLE IF EXISTS `enginebase`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `enginebase` (
  `enginebaseid` int(11) NOT NULL AUTO_INCREMENT,
  `liter` char(6) NOT NULL DEFAULT '',
  `cc` char(5) NOT NULL DEFAULT '',
  `cid` char(3) NOT NULL DEFAULT '',
  `cylinders` char(2) NOT NULL DEFAULT '',
  `blocktype` char(2) NOT NULL DEFAULT '',
  `engborein` char(6) NOT NULL DEFAULT '',
  `engboremetric` char(6) NOT NULL DEFAULT '',
  `engstrokein` char(6) NOT NULL DEFAULT '',
  `engstrokemetric` char(6) NOT NULL DEFAULT '',
  PRIMARY KEY (`enginebaseid`)
) ENGINE=MyISAM AUTO_INCREMENT=17608 DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `engineconfig`
--

DROP TABLE IF EXISTS `engineconfig`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `engineconfig` (
  `engineconfigid` int(11) NOT NULL AUTO_INCREMENT,
  `enginedesignationid` int(11) NOT NULL DEFAULT '0',
  `enginevinid` int(11) NOT NULL DEFAULT '0',
  `valvesid` int(11) NOT NULL,
  `enginebaseid` int(11) NOT NULL DEFAULT '0',
  `fueldeliveryconfigid` int(11) NOT NULL DEFAULT '0',
  `aspirationid` int(11) NOT NULL DEFAULT '0',
  `cylinderheadtypeid` int(11) NOT NULL DEFAULT '0',
  `fueltypeid` int(11) NOT NULL DEFAULT '0',
  `ignitionsystemtypeid` int(11) NOT NULL DEFAULT '0',
  `enginemfrid` int(11) NOT NULL DEFAULT '0',
  `engineversionid` int(11) NOT NULL DEFAULT '0',
  `poweroutputid` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`engineconfigid`),
  KEY `enginebaseid` (`enginebaseid`)
) ENGINE=MyISAM AUTO_INCREMENT=22406 DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `enginedesignation`
--

DROP TABLE IF EXISTS `enginedesignation`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `enginedesignation` (
  `enginedesignationid` int(11) NOT NULL AUTO_INCREMENT,
  `enginedesignationname` varchar(30) NOT NULL DEFAULT '',
  PRIMARY KEY (`enginedesignationid`)
) ENGINE=MyISAM AUTO_INCREMENT=15608 DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `enginelegacy`
--

DROP TABLE IF EXISTS `enginelegacy`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `enginelegacy` (
  `enginelegacyid` int(11) NOT NULL AUTO_INCREMENT,
  `engtype` varchar(10) NOT NULL DEFAULT '',
  `liter` varchar(10) NOT NULL DEFAULT '',
  `cc` varchar(10) NOT NULL DEFAULT '',
  `cid` varchar(10) NOT NULL DEFAULT '',
  `fuel` varchar(15) NOT NULL DEFAULT '',
  `fueldel` varchar(10) NOT NULL DEFAULT '',
  `asp` varchar(20) NOT NULL DEFAULT '',
  `engvin` varchar(10) NOT NULL DEFAULT '',
  `engdesg` varchar(20) NOT NULL DEFAULT '',
  PRIMARY KEY (`enginelegacyid`)
) ENGINE=MyISAM AUTO_INCREMENT=6057 DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `engineversion`
--

DROP TABLE IF EXISTS `engineversion`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `engineversion` (
  `engineversionid` int(11) NOT NULL AUTO_INCREMENT,
  `engineversion` varchar(20) NOT NULL DEFAULT '',
  PRIMARY KEY (`engineversionid`)
) ENGINE=MyISAM AUTO_INCREMENT=178 DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `enginevin`
--

DROP TABLE IF EXISTS `enginevin`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `enginevin` (
  `enginevinid` int(11) NOT NULL AUTO_INCREMENT,
  `enginevinname` varchar(5) NOT NULL DEFAULT '',
  PRIMARY KEY (`enginevinid`)
) ENGINE=MyISAM AUTO_INCREMENT=136 DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `fueldeliveryconfig`
--

DROP TABLE IF EXISTS `fueldeliveryconfig`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `fueldeliveryconfig` (
  `fueldeliveryconfigid` int(11) NOT NULL AUTO_INCREMENT,
  `fueldeliverytypeid` int(11) NOT NULL DEFAULT '0',
  `fueldeliverysubtypeid` int(11) NOT NULL DEFAULT '0',
  `fuelsystemcontroltypeid` int(11) NOT NULL DEFAULT '0',
  `fuelsystemdesignid` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`fueldeliveryconfigid`)
) ENGINE=MyISAM AUTO_INCREMENT=496 DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `fueldeliverysubtype`
--

DROP TABLE IF EXISTS `fueldeliverysubtype`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `fueldeliverysubtype` (
  `fueldeliverysubtypeid` int(11) NOT NULL AUTO_INCREMENT,
  `fueldeliverysubtypename` varchar(50) NOT NULL DEFAULT '',
  PRIMARY KEY (`fueldeliverysubtypeid`)
) ENGINE=MyISAM AUTO_INCREMENT=43 DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `fueldeliverytype`
--

DROP TABLE IF EXISTS `fueldeliverytype`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `fueldeliverytype` (
  `fueldeliverytypeid` int(11) NOT NULL AUTO_INCREMENT,
  `fueldeliverytypename` varchar(50) NOT NULL DEFAULT '',
  PRIMARY KEY (`fueldeliverytypeid`)
) ENGINE=MyISAM AUTO_INCREMENT=7 DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `fuelsystemcontroltype`
--

DROP TABLE IF EXISTS `fuelsystemcontroltype`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `fuelsystemcontroltype` (
  `fuelsystemcontroltypeid` int(11) NOT NULL AUTO_INCREMENT,
  `fuelsystemcontroltypename` varchar(50) NOT NULL DEFAULT '',
  PRIMARY KEY (`fuelsystemcontroltypeid`)
) ENGINE=MyISAM AUTO_INCREMENT=7 DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `fuelsystemdesign`
--

DROP TABLE IF EXISTS `fuelsystemdesign`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `fuelsystemdesign` (
  `fuelsystemdesignid` int(11) NOT NULL AUTO_INCREMENT,
  `fuelsystemdesignname` varchar(50) NOT NULL DEFAULT '',
  PRIMARY KEY (`fuelsystemdesignid`)
) ENGINE=MyISAM AUTO_INCREMENT=165 DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `fueltype`
--

DROP TABLE IF EXISTS `fueltype`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `fueltype` (
  `fueltypeid` int(11) NOT NULL AUTO_INCREMENT,
  `fueltypename` varchar(30) NOT NULL DEFAULT '',
  PRIMARY KEY (`fueltypeid`)
) ENGINE=MyISAM AUTO_INCREMENT=23 DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `ignitionsystemtype`
--

DROP TABLE IF EXISTS `ignitionsystemtype`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `ignitionsystemtype` (
  `ignitionsystemtypeid` int(11) NOT NULL AUTO_INCREMENT,
  `ignitionsystemtypename` varchar(30) NOT NULL DEFAULT '',
  PRIMARY KEY (`ignitionsystemtypeid`)
) ENGINE=MyISAM AUTO_INCREMENT=8 DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `legacyvehicle`
--

DROP TABLE IF EXISTS `legacyvehicle`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `legacyvehicle` (
  `legacyvehicleid` int(11) NOT NULL DEFAULT '0',
  `year` varchar(4) NOT NULL DEFAULT '',
  `make` varchar(50) NOT NULL DEFAULT '',
  `model` varchar(50) NOT NULL DEFAULT '',
  `submodel` varchar(50) NOT NULL DEFAULT '',
  `enginelegacyid` int(11) NOT NULL DEFAULT '0',
  `country` varchar(10) NOT NULL DEFAULT '',
  PRIMARY KEY (`legacyvehicleid`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `make`
--

DROP TABLE IF EXISTS `make`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `make` (
  `makeid` int(11) NOT NULL AUTO_INCREMENT,
  `makename` varchar(50) NOT NULL DEFAULT '',
  PRIMARY KEY (`makeid`),
  KEY `makename` (`makename`)
) ENGINE=MyISAM AUTO_INCREMENT=1325 DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `mfr`
--

DROP TABLE IF EXISTS `mfr`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `mfr` (
  `mfrid` int(11) NOT NULL AUTO_INCREMENT,
  `mfrname` varchar(30) NOT NULL DEFAULT '',
  PRIMARY KEY (`mfrid`)
) ENGINE=MyISAM AUTO_INCREMENT=667 DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `mfrbodycode`
--

DROP TABLE IF EXISTS `mfrbodycode`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `mfrbodycode` (
  `mfrbodycodeid` int(11) NOT NULL AUTO_INCREMENT,
  `mfrbodycodename` varchar(10) NOT NULL DEFAULT '',
  PRIMARY KEY (`mfrbodycodeid`)
) ENGINE=MyISAM AUTO_INCREMENT=4095 DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `model`
--

DROP TABLE IF EXISTS `model`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `model` (
  `modelid` int(11) NOT NULL AUTO_INCREMENT,
  `modelname` varchar(50) NOT NULL DEFAULT '',
  `vehicletypeid` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`modelid`),
  KEY `modelname` (`modelname`)
) ENGINE=MyISAM AUTO_INCREMENT=26347 DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `region`
--

DROP TABLE IF EXISTS `region`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `region` (
  `regionid` int(11) NOT NULL AUTO_INCREMENT,
  `parentid` int(11) DEFAULT NULL,
  `regionabbr` char(3) NOT NULL DEFAULT '',
  `regionname` varchar(30) NOT NULL DEFAULT '',
  PRIMARY KEY (`regionid`)
) ENGINE=MyISAM AUTO_INCREMENT=4 DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `restrainttype`
--

DROP TABLE IF EXISTS `restrainttype`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `restrainttype` (
  `restrainttypeid` int(11) NOT NULL AUTO_INCREMENT,
  `restrainttypename` varchar(50) NOT NULL DEFAULT '',
  PRIMARY KEY (`restrainttypeid`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `springtype`
--

DROP TABLE IF EXISTS `springtype`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `springtype` (
  `springtypeid` int(11) NOT NULL AUTO_INCREMENT,
  `springtypename` varchar(50) NOT NULL DEFAULT '',
  PRIMARY KEY (`springtypeid`)
) ENGINE=MyISAM AUTO_INCREMENT=13 DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `springtypeconfig`
--

DROP TABLE IF EXISTS `springtypeconfig`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `springtypeconfig` (
  `springtypeconfigid` int(11) NOT NULL AUTO_INCREMENT,
  `frontspringtypeid` int(11) NOT NULL DEFAULT '0',
  `rearspringtypeid` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`springtypeconfigid`)
) ENGINE=MyISAM AUTO_INCREMENT=35 DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `steeringconfig`
--

DROP TABLE IF EXISTS `steeringconfig`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `steeringconfig` (
  `steeringconfigid` int(11) NOT NULL AUTO_INCREMENT,
  `steeringtypeid` int(11) NOT NULL DEFAULT '0',
  `steeringsystemid` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`steeringconfigid`)
) ENGINE=MyISAM AUTO_INCREMENT=16 DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `steeringsystem`
--

DROP TABLE IF EXISTS `steeringsystem`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `steeringsystem` (
  `steeringsystemid` int(11) NOT NULL AUTO_INCREMENT,
  `steeringsystemname` varchar(30) NOT NULL DEFAULT '',
  PRIMARY KEY (`steeringsystemid`)
) ENGINE=MyISAM AUTO_INCREMENT=8 DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `steeringtype`
--

DROP TABLE IF EXISTS `steeringtype`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `steeringtype` (
  `steeringtypeid` int(11) NOT NULL AUTO_INCREMENT,
  `steeringtypename` varchar(30) NOT NULL DEFAULT '',
  PRIMARY KEY (`steeringtypeid`)
) ENGINE=MyISAM AUTO_INCREMENT=8 DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `submodel`
--

DROP TABLE IF EXISTS `submodel`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `submodel` (
  `submodelid` int(11) NOT NULL AUTO_INCREMENT,
  `submodelname` varchar(50) NOT NULL DEFAULT '',
  PRIMARY KEY (`submodelid`)
) ENGINE=MyISAM AUTO_INCREMENT=4656 DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `transfercase`
--

DROP TABLE IF EXISTS `transfercase`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `transfercase` (
  `transfercaseid` int(11) NOT NULL AUTO_INCREMENT,
  `transfercasebaseid` int(11) NOT NULL DEFAULT '0',
  `transfercasemfrcode` varchar(30) NOT NULL DEFAULT '',
  `transfercaseelectroniccontrolled` char(3) NOT NULL DEFAULT '',
  `transfercasemfrid` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`transfercaseid`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `transfercasebase`
--

DROP TABLE IF EXISTS `transfercasebase`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `transfercasebase` (
  `transfercasebaseid` int(11) NOT NULL AUTO_INCREMENT,
  `transfercasetype` varchar(30) NOT NULL DEFAULT '',
  `transfercasenumspeeds` char(3) NOT NULL DEFAULT '',
  PRIMARY KEY (`transfercasebaseid`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `transmission`
--

DROP TABLE IF EXISTS `transmission`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `transmission` (
  `transmissionid` int(11) NOT NULL AUTO_INCREMENT,
  `transmissionbaseid` int(11) NOT NULL DEFAULT '0',
  `transmissionmfrcodeid` int(11) NOT NULL DEFAULT '0',
  `transmissionelectroniccontrolled` char(3) NOT NULL DEFAULT '',
  `transmissionmfrid` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`transmissionid`)
) ENGINE=MyISAM AUTO_INCREMENT=5600 DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `transmissionbase`
--

DROP TABLE IF EXISTS `transmissionbase`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `transmissionbase` (
  `transmissionbaseid` int(11) NOT NULL AUTO_INCREMENT,
  `transmissiontypeid` int(11) NOT NULL DEFAULT '0',
  `transmissionnumspeedsid` int(11) NOT NULL DEFAULT '0',
  `transmissioncontroltypeid` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`transmissionbaseid`)
) ENGINE=MyISAM AUTO_INCREMENT=119 DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `transmissioncontroltype`
--

DROP TABLE IF EXISTS `transmissioncontroltype`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `transmissioncontroltype` (
  `transmissioncontroltypeid` int(11) NOT NULL AUTO_INCREMENT,
  `transmissioncontroltypename` varchar(30) NOT NULL DEFAULT '',
  PRIMARY KEY (`transmissioncontroltypeid`)
) ENGINE=MyISAM AUTO_INCREMENT=12 DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `transmissionmfrcode`
--

DROP TABLE IF EXISTS `transmissionmfrcode`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `transmissionmfrcode` (
  `transmissionmfrcodeid` int(11) NOT NULL AUTO_INCREMENT,
  `transmissionmfrcode` varchar(30) NOT NULL DEFAULT '',
  PRIMARY KEY (`transmissionmfrcodeid`)
) ENGINE=MyISAM AUTO_INCREMENT=3583 DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `transmissionnumspeeds`
--

DROP TABLE IF EXISTS `transmissionnumspeeds`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `transmissionnumspeeds` (
  `transmissionnumspeedsid` int(11) NOT NULL AUTO_INCREMENT,
  `transmissionnumspeeds` char(3) NOT NULL DEFAULT '',
  PRIMARY KEY (`transmissionnumspeedsid`)
) ENGINE=MyISAM AUTO_INCREMENT=24 DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `transmissiontype`
--

DROP TABLE IF EXISTS `transmissiontype`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `transmissiontype` (
  `transmissiontypeid` int(11) NOT NULL AUTO_INCREMENT,
  `transmissiontypename` varchar(30) NOT NULL DEFAULT '',
  PRIMARY KEY (`transmissiontypeid`)
) ENGINE=MyISAM AUTO_INCREMENT=8 DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `validattributes`
--

DROP TABLE IF EXISTS `validattributes`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `validattributes` (
  `basevehicleid` int(10) unsigned NOT NULL,
  `attributename` varchar(255) NOT NULL,
  `attributevalue` varchar(255) NOT NULL,
  `acesversiondate` char(10) NOT NULL,
  KEY `basevehicleid` (`basevehicleid`),
  KEY `vidnnamevalue` (`basevehicleid`,`attributename`,`attributevalue`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `valves`
--

DROP TABLE IF EXISTS `valves`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `valves` (
  `valvesid` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `valvesperengine` char(3) NOT NULL DEFAULT '',
  PRIMARY KEY (`valvesid`)
) ENGINE=MyISAM AUTO_INCREMENT=33 DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `vehengcfgtolegacyvehicle`
--

DROP TABLE IF EXISTS `vehengcfgtolegacyvehicle`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `vehengcfgtolegacyvehicle` (
  `vehengcfgtolegacyvehicleid` int(11) NOT NULL AUTO_INCREMENT,
  `vehicletoengineconfigid` int(11) NOT NULL DEFAULT '0',
  `legacyvehicleid` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`vehengcfgtolegacyvehicleid`),
  KEY `vehicletoengineconfigid` (`vehicletoengineconfigid`),
  KEY `legacyvehicleid` (`legacyvehicleid`)
) ENGINE=MyISAM AUTO_INCREMENT=262640 DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `vehicle`
--

DROP TABLE IF EXISTS `vehicle`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `vehicle` (
  `vehicleid` int(11) NOT NULL AUTO_INCREMENT,
  `basevehicleid` int(11) NOT NULL DEFAULT '0',
  `submodelid` int(11) NOT NULL DEFAULT '0',
  `regionid` int(11) NOT NULL,
  `source` varchar(50) DEFAULT NULL,
  `publicationstageid` int(11) NOT NULL,
  `publicationstagesource` varchar(50) NOT NULL,
  PRIMARY KEY (`vehicleid`),
  KEY `basevehicleid` (`basevehicleid`)
) ENGINE=MyISAM AUTO_INCREMENT=245494 DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `vehicleconfig`
--

DROP TABLE IF EXISTS `vehicleconfig`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `vehicleconfig` (
  `VehicleConfigID` int(10) unsigned NOT NULL DEFAULT '0',
  `VehicleID` int(10) unsigned NOT NULL DEFAULT '0',
  `BedConfigID` int(10) unsigned NOT NULL DEFAULT '0',
  `BodyStyleConfigID` int(10) unsigned NOT NULL DEFAULT '0',
  `BrakeConfigID` int(10) unsigned NOT NULL DEFAULT '0',
  `DriveTypeID` int(10) unsigned NOT NULL DEFAULT '0',
  `EngineConfigID` int(10) unsigned NOT NULL DEFAULT '0',
  `MfrBodyCodeID` int(10) unsigned NOT NULL DEFAULT '0',
  `SpringTypeConfigID` int(10) unsigned NOT NULL DEFAULT '0',
  `SteeringConfigID` int(10) unsigned NOT NULL DEFAULT '0',
  `TransmissionID` int(10) unsigned NOT NULL DEFAULT '0',
  `WheelbaseID` int(10) unsigned NOT NULL DEFAULT '0',
  `source` varchar(20) NOT NULL DEFAULT '',
  `date` datetime NOT NULL DEFAULT '0000-00-00 00:00:00',
  PRIMARY KEY (`VehicleConfigID`),
  KEY `VehicleID` (`VehicleID`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `vehicletobedconfig`
--

DROP TABLE IF EXISTS `vehicletobedconfig`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `vehicletobedconfig` (
  `vehicletobedconfigid` int(11) NOT NULL DEFAULT '0',
  `vehicleid` int(11) NOT NULL DEFAULT '0',
  `bedconfigid` int(11) NOT NULL DEFAULT '0',
  `source` char(10) DEFAULT NULL,
  PRIMARY KEY (`vehicletobedconfigid`),
  KEY `vehicleid` (`vehicleid`)
) ENGINE=MyISAM AUTO_INCREMENT=438715 DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `vehicletobodystyleconfig`
--

DROP TABLE IF EXISTS `vehicletobodystyleconfig`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `vehicletobodystyleconfig` (
  `vehicletobodystyleconfigid` int(11) NOT NULL DEFAULT '0',
  `vehicleid` int(11) NOT NULL DEFAULT '0',
  `bodystyleconfigid` int(11) NOT NULL DEFAULT '0',
  `source` char(10) DEFAULT NULL,
  PRIMARY KEY (`vehicletobodystyleconfigid`),
  KEY `vehicleid` (`vehicleid`)
) ENGINE=MyISAM AUTO_INCREMENT=494983 DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `vehicletobrakeconfig`
--

DROP TABLE IF EXISTS `vehicletobrakeconfig`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `vehicletobrakeconfig` (
  `vehicletobrakeconfigid` int(11) NOT NULL,
  `vehicleid` int(11) NOT NULL DEFAULT '0',
  `brakeconfigid` int(11) NOT NULL DEFAULT '0',
  `source` varchar(10) NOT NULL,
  PRIMARY KEY (`vehicletobrakeconfigid`),
  KEY `vehicleid` (`vehicleid`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `vehicletodrivetype`
--

DROP TABLE IF EXISTS `vehicletodrivetype`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `vehicletodrivetype` (
  `vehicletodrivetypeid` int(11) NOT NULL DEFAULT '0',
  `vehicleid` int(11) NOT NULL DEFAULT '0',
  `drivetypeid` int(11) NOT NULL DEFAULT '0',
  `source` char(10) DEFAULT NULL,
  PRIMARY KEY (`vehicletodrivetypeid`),
  KEY `vehicleid` (`vehicleid`)
) ENGINE=MyISAM AUTO_INCREMENT=432107 DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `vehicletoengineconfig`
--

DROP TABLE IF EXISTS `vehicletoengineconfig`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `vehicletoengineconfig` (
  `vehicletoengineconfigid` int(11) NOT NULL,
  `vehicleid` int(11) NOT NULL DEFAULT '0',
  `engineconfigid` int(11) NOT NULL DEFAULT '0',
  `source` char(10) DEFAULT NULL,
  PRIMARY KEY (`vehicletoengineconfigid`),
  KEY `vehicleid` (`vehicleid`),
  KEY `engineconfigid` (`engineconfigid`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `vehicletomfrbodycode`
--

DROP TABLE IF EXISTS `vehicletomfrbodycode`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `vehicletomfrbodycode` (
  `vehicletomfrbodycodeid` int(11) NOT NULL DEFAULT '0',
  `vehicleid` int(11) NOT NULL DEFAULT '0',
  `mfrbodycodeid` int(11) NOT NULL DEFAULT '0',
  `source` char(10) DEFAULT NULL,
  PRIMARY KEY (`vehicletomfrbodycodeid`),
  KEY `vehicleid` (`vehicleid`)
) ENGINE=MyISAM AUTO_INCREMENT=533757 DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `vehicletospringtypeconfig`
--

DROP TABLE IF EXISTS `vehicletospringtypeconfig`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `vehicletospringtypeconfig` (
  `vehicletospringtypeconfigid` int(11) NOT NULL DEFAULT '0',
  `vehicleid` int(11) NOT NULL DEFAULT '0',
  `springconfigid` int(11) DEFAULT NULL,
  `source` char(10) DEFAULT NULL,
  PRIMARY KEY (`vehicletospringtypeconfigid`),
  KEY `vehicleid` (`vehicleid`)
) ENGINE=MyISAM AUTO_INCREMENT=188261 DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `vehicletosteeringconfig`
--

DROP TABLE IF EXISTS `vehicletosteeringconfig`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `vehicletosteeringconfig` (
  `vehicletosteeringconfigid` int(11) NOT NULL DEFAULT '0',
  `vehicleid` int(11) NOT NULL DEFAULT '0',
  `steeringconfigid` int(11) NOT NULL DEFAULT '0',
  `source` char(10) DEFAULT NULL,
  PRIMARY KEY (`vehicletosteeringconfigid`),
  KEY `vehicleid` (`vehicleid`)
) ENGINE=MyISAM AUTO_INCREMENT=415169 DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `vehicletotransmission`
--

DROP TABLE IF EXISTS `vehicletotransmission`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `vehicletotransmission` (
  `vehicletotransmissionid` int(11) NOT NULL DEFAULT '0',
  `vehicleid` int(11) NOT NULL DEFAULT '0',
  `transmissionid` int(11) NOT NULL DEFAULT '0',
  `source` char(10) DEFAULT NULL,
  PRIMARY KEY (`vehicletotransmissionid`),
  KEY `vehicleid` (`vehicleid`)
) ENGINE=MyISAM AUTO_INCREMENT=727068 DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `vehicletowheelbase`
--

DROP TABLE IF EXISTS `vehicletowheelbase`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `vehicletowheelbase` (
  `vehicletowheelbaseid` int(11) NOT NULL DEFAULT '0',
  `vehicleid` int(11) NOT NULL DEFAULT '0',
  `wheelbaseid` int(11) NOT NULL DEFAULT '0',
  `source` char(10) DEFAULT NULL,
  PRIMARY KEY (`vehicletowheelbaseid`),
  KEY `vehicleid` (`vehicleid`)
) ENGINE=MyISAM AUTO_INCREMENT=459726 DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `vehicletype`
--

DROP TABLE IF EXISTS `vehicletype`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `vehicletype` (
  `vehicletypeid` int(11) NOT NULL AUTO_INCREMENT,
  `vehicletypename` varchar(50) NOT NULL DEFAULT '',
  `vehicletypegroupid` tinyint(3) unsigned NOT NULL,
  PRIMARY KEY (`vehicletypeid`)
) ENGINE=MyISAM AUTO_INCREMENT=2202 DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `version`
--

DROP TABLE IF EXISTS `version`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `version` (
  `versiondate` date NOT NULL DEFAULT '0000-00-00'
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `wheelbase`
--

DROP TABLE IF EXISTS `wheelbase`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `wheelbase` (
  `wheelbaseid` int(11) NOT NULL AUTO_INCREMENT,
  `wheelbase` varchar(10) NOT NULL DEFAULT '',
  `wheelbasemetric` varchar(10) NOT NULL DEFAULT '',
  PRIMARY KEY (`wheelbaseid`)
) ENGINE=MyISAM AUTO_INCREMENT=477 DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `year`
--

DROP TABLE IF EXISTS `year`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `year` (
  `yearid` int(11) NOT NULL AUTO_INCREMENT,
  PRIMARY KEY (`yearid`)
) ENGINE=MyISAM AUTO_INCREMENT=2019 DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

-- Dump completed on 2017-01-28 10:54:17
