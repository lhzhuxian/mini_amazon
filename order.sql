delete from uorder;
delete from aorder;
INSERT INTO aorder (ordernum, userid, id, description, amount, whid, desx, desy, truck_ready, pack_ready, load_ready, delivered)
VALUES (1, 1, 1, 'book', 1, 0, 1, 1, FALSE, FALSE, FALSE, FALSE);
INSERT INTO aorder (ordernum, userid, id, description, amount, whid, desx, desy, truck_ready, pack_ready, load_ready, delivered)
VALUES (2, 1, 1, 'book', 5, 1, 5, 2, FALSE, FALSE, FALSE, FALSE);
INSERT INTO aorder (ordernum, userid, id, description, amount, whid, desx, desy, truck_ready, pack_ready, load_ready, delivered)
VALUES (3, 1, 1, 'book', 10, 2, 4, 5, FALSE, FALSE, FALSE, FALSE);
INSERT INTO aorder (ordernum, userid, id, description, amount, whid, desx, desy, truck_ready, pack_ready, load_ready, delivered)
VALUES (4, 1, 1, 'book', 100, 0, 2, 177, FALSE, FALSE, FALSE, FALSE);
INSERT INTO aorder (ordernum, userid, id, description, amount, whid, desx, desy, truck_ready, pack_ready, load_ready, delivered)
VALUES (5, 3, 2, 'pen', 100, 0, 2, 177, FALSE, FALSE, FALSE, FALSE);
INSERT INTO aorder (ordernum, userid, id, description, amount, whid, desx, desy, truck_ready, pack_ready, load_ready, delivered)
VALUES (6, 1, 2, 'pen', 100, 1, 2, 177, FALSE, FALSE, FALSE, FALSE);
