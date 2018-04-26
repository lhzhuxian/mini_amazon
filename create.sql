drop table if exists account, good, stock, aorder, uorder;
CREATE TABLE ACCOUNT(           
  ID   INT PRIMARY KEY,
  email TEXT NOT NULL);

CREATE TABLE GOOD(
  DESCRIPTION   TEXT   NOT NULL, 
  ID    INT    PRIMARY KEY);
  
CREATE TABLE STOCK(
  ID   	INT NOT NULL,
  WHID  INT   NOT NULL,
  AMOUNT  INT    NOT NULL);

CREATE TABLE WAREHOUSE(
  WHID    INT NOT NULL,
  X  INT   NOT NULL,
  Y  INT    NOT NULL);
}
  
CREATE TABLE AORDER(
  ORDERNUM INT NOT NULL,
  USERID INT NOT NULL,
  ID INT NOT NULL,
  DESCRIPTION TEXT NOT NULL,
  AMOUNT INT NOT NULL,
  UPS TEXT,
  WHID INT NOT NULL,
  DESX INT NOT NULL,
  DESY INT NOT NULL,
  GOODID INT,
  TRUCKID INT,
  TRUCK_READY BOOLEAN NOT NULL,
  PACK_READY BOOLEAN NOT NULL,
  LOAD_READY BOOLEAN NOT NULL,
  DELIVERED BOOLEAN NOT NULL);

CREATE TABLE UORDER(
  ORDERNUM INT NOT NULL,
  ID INT NOT NULL,
  DESCRIPTION TEXT NOT NULL,
  AMOUNT INT NOT NULL,
  UPS TEXT,
  WHID INT NOT NULL,
  DESX INT NOT NULL,
  DESY INT NOT NULL);




