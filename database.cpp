#include "database.hpp"


database::database(int wout, int uout)
  : c(new connection("dbname=amazon user=postgres password = passw0rd"))
  , order(0)
  , wh_out(new socket_out(wout))
  , ups_out(new socket_out(uout))
{ }

database::~database() {
  delete c;
  delete wh_out;
  delete ups_out;
}
result database::run_query(string &sql) {
  nontransaction N(*c);
  result R(N.exec(sql));
  return R;
}
result database::run_command(string &sql) {
    work W(*c);
    result R(W.exec(sql));
    W.commit();
    return R;
}

void database::add_buy_command(ACommands &WarehouseRequest, int whnum, int id, string &description, int count){
  APurchaseMore * PurchaseOrder = WarehouseRequest.add_buy();
  PurchaseOrder->set_whnum(whnum);
  AProduct * Product = PurchaseOrder->add_things();
  Product->set_id(id);
  Product->set_description(description);
  Product->set_count(count);
}

void database::add_load_command(ACommands &WarehouseRequest, int whnum, int truckid, int shipid) {
  APutOnTruck * LoadCommand = WarehouseRequest.add_load();
  LoadCommand->set_whnum(whnum);
  LoadCommand->set_truckid(truckid);
  LoadCommand->set_shipid(shipid);
}

void database::add_goodReady_command(A2UResponses &UpsRequest, int whid, string username, \
			   int dex, int dey, int ordernum, int id, \
			   string description, int count){
  AGoodInfo * GoodReady = UpsRequest.add_goodready();
  GoodReady->set_whid(whid);
  if (username.size() == 0) {
    GoodReady->set_username(username);
  }
  GoodReady->set_xdest(dex);
  GoodReady->set_ydest(dey);
  GoodReady->set_order_num(ordernum);
  AProduct * product = GoodReady->add_good();
  product->set_id(id);
  product->set_description(description);
  product->set_count(count);
}

int database::get_order(ACommands &WarehouseRequest, A2UResponses &UpsRequest) {
  int ans;
  string sql = "SELECT * FROM AORDER WHERE ORDERNUM > " + to_string(order) + ";";
  result OrderToDeal = run_query(sql);
  order += OrderToDeal.size();
  if((ans = OrderToDeal.size()) == 0) {
    return ans;
  }
  
  for (auto it = OrderToDeal.begin(); it != OrderToDeal.end(); ++it) {
    int ordernum = it[0].as<int>();
    int id = it[2].as<int>();
    string description = it[3].as<string>();
    int amount = it[4].as<int>();
    string ups;
    if (!it[5].is_null()){
      ups = it[5].as<string>(); // check
    }
    int whid = it[6].as<int>();
    int dex = it[7].as<int>();
    int dey = it[8].as<int>();
    string sql = "SELECT AMOUNT, WHID FROM STOCK WHERE ID = " + to_string(id) \
      + " AND WHID = " + to_string(whid) + ";";
    result stock = run_query(sql);
    if (stock.empty() || stock[0][0].as<int>() < amount) {
      // buy the stuff

      insert_unfulorder(ordernum, id, description, amount, ups, whid, dex, dey);
      add_buy_command(WarehouseRequest, whid, id, description, amount * 100);
      // comm.add_buy
    } else {
      add_goodReady_command(UpsRequest, whid, ups, dex, dey, ordernum, id, description, amount);
      update_stock(id, whid, amount);
    }
  }
  return ans;
}

void database::insert_unfulorder(int ordernum, int id, string &description, int amount, string ups, int whid, int desx, int desy) {
  if (ups.size() == 0) {ups = "NULL";}
  string sql = "INSERT INTO UORDER " \
    "VALUES ( " + to_string(ordernum) + ", " + to_string(id) + ", '" + description + "', " \
    + to_string(amount) + ", '" + ups + "', " + to_string(whid) + ", " + to_string(desx) + ", " \
    + to_string(desy) + ");";
  run_command(sql);
}

void database::update_stock(int id, int whid, int amount) {
  string sql =  "UPDATE STOCK "                                        \
    "SET  AMOUNT = AMOUNT + " + to_string(amount) +  " WHERE id = " + to_string(id) + " AND WHID = " + to_string(whid) + ";";
  run_command(sql);  
}

// need to lock
void database::deal_stock_arrived ( A2UResponses &UpsRequest, int s_id, int s_whid, int s_amount) {
  string sql = "SELECT * FROM UORDER WHERE id =  " + to_string(s_id) + " AND WHID = " \
    + to_string(s_whid) + ";";
  result OrderToDeal = run_query(sql);
  string delete_order = "DELETE FROM UORDER WHERE ";
  int flag = 0;
  for (auto it = OrderToDeal.begin(); it != OrderToDeal.end(); it++) {
    int ordernum = it[0].as<int>();
    int id = it[1].as<int>();
    string description = it[2].as<string>();
    int amount = it[3].as<int>();
    string ups;
    if (!it[4].is_null()){
      ups = it[4].as<string>(); // check
    }
    int whid = it[5].as<int>();
    int dex = it[6].as<int>();
    int dey = it[7].as<int>();
    if (s_amount >= amount) {
      s_amount -= amount;
      if (flag == 0) {
	delete_order += "ORDERNUM = " + to_string(ordernum) + " ";
      } else {
	delete_order += "OR ORDERNUM = " + to_string(ordernum) + " ";
      }
      flag = 1;
      add_goodReady_command(UpsRequest, whid, ups, dex, dey, ordernum, id, description, amount);
    }
    
  }
  cout << "remain stock " << s_amount << endl;
  string command =  "UPDATE STOCK "					\
    "SET  AMOUNT = AMOUNT + " + to_string(s_amount) +  " WHERE id = " + to_string(s_id) + " AND WHID = " + to_string(s_whid) + ";";
  if (flag) {
    delete_order += ";";
    command += delete_order;
  }
  run_command(command);

}

// deal with truckReady response from UPS
void database::deal_truckReady(int goodid, int truckid, int order_num, ACommands & WarehouseRequest) {
 
  string query = "SELECT WHID, ID, DESCRIPTION, AMOUNT FROM AORDER WHERE ordernum = "  + to_string(order_num) + " ;";
  result R = run_query(query);

  // send warehouse a pack command
  APack * PackCommand = WarehouseRequest.add_topack();
  PackCommand->set_whnum(R[0][0].as<int>());
  PackCommand->set_shipid(goodid); 
  AProduct * thing = PackCommand->add_things();
  thing->set_id(R[0][1].as<int>());
  thing->set_description(R[0][2].as<string>());
  thing->set_count(R[0][3].as<int>());
  string sql =  "UPDATE AORDER "                                        \
    "SET  TRUCKID = " + to_string(truckid) + " , GOODID = " + to_string(goodid) + " WHERE ordernum =\
 " + to_string(order_num) + " ;";
  run_command(sql);

}

// deal with ready response from warehouse
void database::deal_pack_ready(int goodid, ACommands & WarehouseRequest) {
 
  string query = "SELECT TRUCK_READY, WHID, TRUCKID FROM AORDER WHERE GOODID = " + to_string(goodid) + ";";
  result R = run_query(query);
  if (R[0][0].as<bool>()) {
    add_load_command(WarehouseRequest, R[0][1].as<int>(),R[0][2].as<int>(), goodid);    
  }
  string sql =  "UPDATE AORDER "                                        \
    "SET  PACK_READY = TRUE WHERE GOODID = " + to_string(goodid) + " ;";
  run_command(sql);
  
}

// deal with truckArrived response from UPS
void database::deal_truckArrived(int truckid, int whid, ACommands & WarehouseRequest) {
 
  string query  = "SELECT PACK_READY, GOODID FROM AORDER WHERE DELIVERED = FALSE AND TRUCKID = " + to_string(truckid) + " AND WHID = " + to_string(whid) + ";";
  result R = run_query(query);

  for (auto it = R.begin(); it != R.end(); it++) {
    if (it[0].as<bool>()) {
      add_load_command(WarehouseRequest, whid, truckid, it[1].as<int>());
    }
  }
  string sql =  "UPDATE AORDER "                                        \
     "SET  TRUCK_READY = TRUE WHERE DELIVERED = FALSE AND TRUCKID = " + to_string(truckid) + " AND WHID = " + to_string(whid) + " ;";
 run_command(sql);
  
}

// deal with load response from warehouse
void database::deal_loaded(int goodid, A2UResponses &UpsRequest) {
  string sql =  "UPDATE AORDER "                                        \
     "SET  LOAD_READY = TRUE WHERE GOODID = " + to_string(goodid) + " ;";
  run_command(sql);
  
  string tmp1 = "SELECT TRUCKID, WHID FROM AORDER WHERE GOODID = " + to_string(goodid) + " ;";
  result tmp2 = run_query(tmp1);
  int truckid = tmp2[0][0].as<int>();
  int whid = tmp2[0][1].as<int>();
  string query = "SELECT * FROM AORDER WHERE DELIVERED = FALSE AND TRUCKID = " + to_string(truckid) \
    + " AND WHID = " + to_string(whid) + " AND LOAD_READY = FALSE " + ";";
  result R = run_query(query);
  if (R.size() == 0) {
    UTruckInfo * LeaveTruck = UpsRequest.add_truckleave();
    LeaveTruck->set_truckid(truckid);
    LeaveTruck->set_whid(whid);
  }
}

//deal with packageFinished response from ups
void database::deal_packageFinished(int goodid) {
  string sql =  "UPDATE AORDER "                                        \
    "SET  DELIVERED = TRUE WHERE GOODID = " + to_string(goodid) + " ;";
  run_command(sql);
}
  



