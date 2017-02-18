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
-- Table structure for table `category`
--

DROP TABLE IF EXISTS `category`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `category` (
  `categoryid` int(6) unsigned NOT NULL,
  `categoryname` varchar(255) NOT NULL,
  PRIMARY KEY (`categoryid`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `codemaster`
--

DROP TABLE IF EXISTS `codemaster`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `codemaster` (
  `codemasterid` int(6) unsigned NOT NULL,
  `partterminologyid` int(6) unsigned DEFAULT NULL,
  `categoryid` int(6) unsigned DEFAULT NULL,
  `subcategoryid` int(6) unsigned DEFAULT NULL,
  `positionid` int(6) unsigned DEFAULT NULL,
  PRIMARY KEY (`codemasterid`),
  KEY `partterminologyid` (`partterminologyid`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `parts`
--

DROP TABLE IF EXISTS `parts`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `parts` (
  `partterminologyid` int(6) unsigned NOT NULL,
  `partterminologyname` varchar(255) DEFAULT NULL,
  `partsdescriptionid` int(6) unsigned DEFAULT NULL,
  PRIMARY KEY (`partterminologyid`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `partsdescription`
--

DROP TABLE IF EXISTS `partsdescription`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `partsdescription` (
  `partsdescriptionid` int(6) unsigned NOT NULL,
  `partsdescription` text,
  PRIMARY KEY (`partsdescriptionid`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `partstouse`
--

DROP TABLE IF EXISTS `partstouse`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `partstouse` (
  `partterminologyid` int(6) unsigned NOT NULL,
  `useid` int(6) unsigned DEFAULT NULL,
  PRIMARY KEY (`partterminologyid`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `position`
--

DROP TABLE IF EXISTS `position`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `position` (
  `positionid` int(6) unsigned NOT NULL,
  `positionname` varchar(255) NOT NULL,
  PRIMARY KEY (`positionid`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `subcategory`
--

DROP TABLE IF EXISTS `subcategory`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `subcategory` (
  `subcategoryid` int(6) unsigned NOT NULL,
  `subcategoryname` varchar(255) NOT NULL,
  PRIMARY KEY (`subcategoryid`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `use`
--

DROP TABLE IF EXISTS `use`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `use` (
  `useid` int(6) unsigned NOT NULL,
  `usedescription` varchar(255) DEFAULT NULL,
  PRIMARY KEY (`useid`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `version`
--

DROP TABLE IF EXISTS `version`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `version` (
  `versiondate` date NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1;
