#include "Common.hpp"
#include "client.hpp"
#include "database.hpp"
#include "server.hpp"

// ignore ups_out for test
void order_task(socket_out * wh_out, socket_out * ups_out,  database * AmazonDB) {
  while(true) {
    ACommands WarehouseRequest;
    A2UResponses UpsRequest;
    int OrderNum = AmazonDB->get_order(WarehouseRequest, UpsRequest);
    if (OrderNum == 0) {

      std::this_thread::sleep_for(std::chrono::milliseconds(10));
      
    } else {
      WarehouseRequest.set_simspeed(1000000);
      if (!sendMesgTo<ACommands> (WarehouseRequest, wh_out) ) {
	cerr << "failed to connect to warehouse\n";
      }
      /*
      if (!sendMesgTo<A2UResponses> (UpsRequest, ups_out) ) {
	cerr << "failed to connect to warehouse\n";
      }
      */
    } 
  }  
}

void ups_task(U2AResponses UpsResponse, database * AmazonDB, socket_out * wh_out,  socket_out * ups_out) {
  ACommands WarehouseRequest;
  A2UResponses UpsRequest;
  for (int j = 0; j < UpsResponse.truckarrived_size(); j++) {
    int truckid = UpsResponse.truckarrived(j).truckid();
    int whid = UpsResponse.truckarrived(j).whid();
    cout << "Truck " << truckid << " has arrived at warehouse " << whid << endl;
    AmazonDB->deal_truckArrived(truckid, whid, WarehouseRequest);
  }
  
  for (int j = 0; j < UpsResponse.packagefinished_size(); j++) {
    int goodid = UpsResponse.packagefinished(j);
    cout << "shipid " << goodid << " has been delivered\n";
    AmazonDB->deal_packageFinished(goodid);
  }
  
  for (int j = 0; j < UpsResponse.truckready_size(); j++) {
    int truckid = UpsResponse.truckready(j).truckid();
    int goodid = UpsResponse.truckready(j).goodid();
    int order_num = UpsResponse.truckready(j).order_num();
    cout << "Order " << order_num << " has been matched to shipid " \
	 << goodid << " and truck " << truckid << endl;
    AmazonDB->deal_truckReady(goodid, truckid, order_num, WarehouseRequest);
  }

  if (UpsResponse.has_error()) {
    cerr << "Here Comes the error from Ups:\n";
    cerr << UpsResponse.error() << endl;
  }

   if (!sendMesgTo<ACommands> (WarehouseRequest, wh_out) ) {
    cerr << "failed sending request to warehouse when dealing with response from warehous\n";
   }
  /*
  if (!sendMesgTo<A2UResponses> (UpsRequest, ups_out) ) {
     cerr << "failed sending request to UPS when dealing with response from warehou\
se\n";
  }
  */


}


void wh_task(AResponses WarehouseResponse, database * AmazonDB, socket_out * wh_out,  socket_out * ups_out) {
  ACommands WarehouseRequest;
  A2UResponses UpsRequest;
  for (int j = 0; j < WarehouseResponse.arrived_size(); j++) {
    const APurchaseMore & arrived = WarehouseResponse.arrived(j);
    for (int i = 0; i < arrived.things_size(); i++) {
      int id = arrived.things(i).id();
      int whnum =  arrived.whnum();
      int amount = arrived.things(i).count();
      cout << amount << " item " << id << " arrived at Warehouse " << whnum << endl;
      AmazonDB->deal_stock_arrived(UpsRequest, id, whnum, amount);
    }
  }
  for (int j = 0; j < WarehouseResponse.ready_size(); ++j) {
    int shipid = WarehouseResponse.ready(j);
    cout << "Shipid " << shipid << " has been packed\n";
    AmazonDB->deal_pack_ready(WarehouseResponse.ready(j), WarehouseRequest);
  }
  
  for (int j = 0; j < WarehouseResponse.loaded_size(); ++j) {
    int shipid = WarehouseResponse.loaded(j);
    cout << "Shipid " << shipid << " has been loaded\n";
    AmazonDB->deal_loaded(WarehouseResponse.loaded(j), UpsRequest);
  }
  if (WarehouseResponse.has_error()) {
    cerr << "Here Comes the error from warehoust:\n";
    cerr << WarehouseResponse.error() << endl;
  }
  if (!sendMesgTo<ACommands> (WarehouseRequest, wh_out) ) {
    cerr << "failed sending request to warehouse when dealing with response from warehouse\n";
  }
  /*
  if (!sendMesgTo<A2UResponses> (UpsRequest, ups_out) ) {
     cerr << "failed sending request to UPS when dealing with response from warehou\
se\n";
  }
  */
  
}



int main(void) {

  // waiting for UPS

  /*
  Server Amazon((char*)"6666");
  int u_sockfd = Amazon.to_accept();
  
  */
  int u_sockfd = 1;
  unique_ptr<socket_out> ups_out(new socket_out(u_sockfd));
  unique_ptr<socket_in> ups_in(new socket_in(u_sockfd));
  
  
   
  // connect to sim_world
  Client wh((char*)"vcm-3223.vm.duke.edu", (char*)"23456");
  int result = wh.to_connect();
  if (result == -1) {
    perror("connect failed\n");
  }
  int w_sockfd = wh.get_sockfd();
  AConnect ConnectRequest;
  AConnected ConnectResponse;
  unique_ptr<socket_out> wh_out(new socket_out(w_sockfd));
  unique_ptr<socket_in> wh_in(new socket_in(w_sockfd));

  unique_ptr<database> AmazonDB (new database(w_sockfd, u_sockfd));
 
  ConnectRequest.set_worldid(1000);
  // init warehouse
  /*
  AInitWarehouse * warehouse = ConnectRequest.add_initwh();
  warehouse->set_x(-664);
  warehouse->set_y(-1081);
  */ 
  if (!sendMesgTo<AConnect> (ConnectRequest, wh_out.get()) ) {
    cerr << "failed to connect to warehouse\n";
  }
  if (!recvMesgFrom<AConnected> (ConnectResponse, wh_in.get()) ) {
    cout << "failed to connect to warehouse\n";
  } else if (ConnectResponse.has_error()) {
    cout << ConnectResponse.error();
  }
  cout << "Connected to the warehouse\n";
  
  // create a thread to handle the order from the frontend
  std::thread order_handler(order_task, wh_out.get(), ups_out.get(),  AmazonDB.get());
  if(order_handler.joinable()) {
    cout << "Detaching Thread" << endl;
    order_handler.detach();
  }
  
  // epoll or select two fd
  fd_set readfds;
  FD_ZERO(&readfds);
  FD_SET(w_sockfd, &readfds);
  //FD_SET(u_sockfd, &readfds);
  int numfds = w_sockfd > u_sockfd ? w_sockfd + 1 : u_sockfd + 1;
  while(true) {
    fd_set tmp = readfds;
    if (select(numfds, &tmp, NULL, NULL, NULL) == -1) {
      perror("select");
      exit(4);
    }
    if (FD_ISSET(w_sockfd, &tmp)) {
      AResponses WarehouseResponse;
      recvMesgFrom<AResponses> (WarehouseResponse, wh_in.get());
      wh_task(WarehouseResponse, AmazonDB.get(), wh_out.get(), ups_out.get());
    }
    /*
    if (FD_ISSET(u_sockfd, &tmp)){
      U2AResponses UpsResponse;
      recvMesgFrom<U2AResponses> (UpsResponse, ups_in.get());
      ups_task(UpsResponse, AmazonDB.get(), wh_out.get(), ups_out.get());
      
     }
    */
  }
  
}
