#ifndef _DATABASE_H__
#define _DATABASE_H__

#include "Common.hpp"


class database {
private:
  connection * c;
  int order;
  socket_out * wh_out;
  socket_out * ups_out;
public:
  database(int wout, int uout);
  ~database();
  result run_query(string &sql);
  result run_command(string &sql);
  void add_buy_command(ACommands &WarehouseRequest, int whnum, int id, string &description, int count);
  void add_load_command(ACommands &WarehouseRequest, int whnum, int truckid, int shipid);
  void add_goodReady_command(A2UResponses &UpsRequest, int whid, string username,\
                           int dex, int dey, int ordernum, int id, \
			     string description, int count);
  int get_order(ACommands &WarehouseRequest, A2UResponses &UpsRequest);
  void insert_unfulorder (int ordernum, int id, string &description, int amount, string ups, int whid, int desx, int desy);
  void update_stock(int id, int whid, int amount);

  void deal_truckReady (int goodid, int truckid, int order_num, ACommands & \
			WarehouseRequest);
  void deal_stock_arrived (A2UResponses &UpsRequest, int s_id, int s_whid, int s_amount);
  void deal_pack_ready(int goodid, ACommands & WarehouseRequest);
  void deal_truckArrived(int truckid, int whid, ACommands & WarehouseRequest);
  void deal_loaded(int goodid, A2UResponses &UpsRequest);
  void deal_packageFinished(int goodid);

};

#endif
