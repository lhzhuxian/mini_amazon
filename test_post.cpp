#include <iostream>
#include <string>
#include <pqxx/pqxx>

using namespace std;
using namespace pqxx;


result run_query(string &sql, connection * c) {
  nontransaction N(*c);
  result R(N.exec(sql));
  return R;
}

int main(void) {
  connection * c = new connection("dbname=amazon user=postgres password = passw0rd");
  string sql = "SELECT * FROM AORDER;";
  result OrderToDeal = run_query(sql, c);
  for (auto it = OrderToDeal.begin(); it != OrderToDeal.end(); ++it) {
    int ordernum = it[0].as<int>();
    int userid = it[1].as<int>();
    int id = it[2].as<int>();
    string description = it[3].as<string>();
    int amount = it[4].as<int>();
    string ups;
    
    if (!it[5].is_null()) {
      ups = it[5].as<string>();
    }
    
    int whid = it[6].as<int>();
    int dex = it[7].as<int>();
    int dey = it[8].as<int>();
    
    cout << "come: " << endl;
    cout << ups.size() << endl;
    cout << ups << endl;
    
  } 

}
