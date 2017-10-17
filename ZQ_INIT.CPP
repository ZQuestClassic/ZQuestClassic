//--------------------------------------------------------
//  Zelda Classic
//  by Jeremy Craner, 1999-2000
//
//  zquest.cc
//
//  Main code for the quest editor.
//
//--------------------------------------------------------

#include "zc_alleg.h"
#include "zdefs.h"
#include "jwin.h"
#include "zsys.h"
#include <stdio.h>
#include "gui.h"

extern int d_dummy_proc(int msg,DIALOG *d,int c);
extern int d_dropdmaplist_proc(int msg,DIALOG *d,int c);
extern int onHelp();
extern FONT *lfont;
extern int jwin_pal[jcMAX];
extern char *dmaplist(int index, int *list_size);
extern bool saved;
extern int dmap_list_size;
extern bool dmap_list_zero;
zinitdata zinit;
extern int startdmapxy[6];





int d_line_proc(int msg, DIALOG *d, int c)
{
  if (msg==MSG_DRAW)
  {
    int fg = (d->flags & D_DISABLED) ? gui_mg_color : d->fg;
    line(screen, d->x, d->y, d->x+d->w, d->y+d->h, fg);
  }
  return D_O_K;
}

static int init_equipment_list[] =
{
  // dialog control number
  5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, -1
};

static int init_items_list[] =
{
  // dialog control number
  65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 128, 129, 130, 131, 132, 133, 134, 135, 136, -1
};

static int init_dmap_items_list[] =
{
  // dialog control number
  137, -1
};

static int init_dmap_items_000s_list[] =
{
  // dialog control number
  138, -1
};

static int init_dmap_items_100s_list[] =
{
  // dialog control number
  139, -1
};

static int init_dmap_items_200s_list[] =
{
  // dialog control number
  140, -1
};

static int init_dmap_items_0_00s_list[] =
{
  // dialog control number
  141, 167, 168, 219, 220, 271, 272, 323, 324, 375, 376, 377, 378, 379, 380, 381, 382, 383, 384, 631, 632, 633, 634, 635, 636, 637, 638, 639, 640, 887, 888, 889, 890, 891, 892, 893, 894, 895, 896, 1143, 1144, 1145, 1146, 1147, 1148, 1149, 1150, 1151, 1152, 1399, 1400, 1401, 1402, 1403, 1404, 1405, 1406, 1407, 1408, -1
};

static int init_dmap_items_0_10s_list[] =
{
  // dialog control number
  142, 169, 170, 221, 222, 273, 274, 325, 326, 385, 386, 387, 388, 389, 390, 391, 392, 393, 394, 641, 642, 643, 644, 645, 646, 647, 648, 649, 650, 897, 898, 899, 900, 901, 902, 903, 904, 905, 906, 1153, 1154, 1155, 1156, 1157, 1158, 1159, 1160, 1161, 1162, 1409, 1410, 1411, 1412, 1413, 1414, 1415, 1416, 1417, 1418, -1
};

static int init_dmap_items_0_20s_list[] =
{
  // dialog control number
  143, 171, 172, 223, 224, 275, 276, 327, 328, 395, 396, 397, 398, 399, 400, 401, 402, 403, 404, 651, 652, 653, 654, 655, 656, 657, 658, 659, 660, 907, 908, 909, 910, 911, 912, 913, 914, 915, 916, 1163, 1164, 1165, 1166, 1167, 1168, 1169, 1170, 1171, 1172, 1419, 1420, 1421, 1422, 1423, 1424, 1425, 1426, 1427, 1428, -1
};

static int init_dmap_items_0_30s_list[] =
{
  // dialog control number
  144, 173, 174, 225, 226, 277, 278, 329, 330, 405, 406, 407, 408, 409, 410, 411, 412, 413, 414, 661, 662, 663, 664, 665, 666, 667, 668, 669, 670, 917, 918, 919, 920, 921, 922, 923, 924, 925, 926, 1173, 1174, 1175, 1176, 1177, 1178, 1179, 1180, 1181, 1182, 1429, 1430, 1431, 1432, 1433, 1434, 1435, 1436, 1437, 1438, -1
};

static int init_dmap_items_0_40s_list[] =
{
  // dialog control number
  145, 175, 176, 227, 228, 279, 280, 331, 332, 415, 416, 417, 418, 419, 420, 421, 422, 423, 424, 671, 672, 673, 674, 675, 676, 677, 678, 679, 680, 927, 928, 929, 930, 931, 932, 933, 934, 935, 936, 1183, 1184, 1185, 1186, 1187, 1188, 1189, 1190, 1191, 1192, 1439, 1440, 1441, 1442, 1443, 1444, 1445, 1446, 1447, 1448, -1
};

static int init_dmap_items_0_50s_list[] =
{
  // dialog control number
  146, 177, 178, 229, 230, 281, 282, 333, 334, 425, 426, 427, 428, 429, 430, 431, 432, 433, 434, 681, 682, 683, 684, 685, 686, 687, 688, 689, 690, 937, 938, 939, 940, 941, 942, 943, 944, 945, 946, 1193, 1194, 1195, 1196, 1197, 1198, 1199, 1200, 1201, 1202, 1449, 1450, 1451, 1452, 1453, 1454, 1455, 1456, 1457, 1458, -1
};

static int init_dmap_items_0_60s_list[] =
{
  // dialog control number
  147, 179, 180, 231, 232, 283, 284, 335, 336, 435, 436, 437, 438, 439, 440, 441, 442, 443, 444, 691, 692, 693, 694, 695, 696, 697, 698, 699, 700, 947, 948, 949, 950, 951, 952, 953, 954, 955, 956, 1203, 1204, 1205, 1206, 1207, 1208, 1209, 1210, 1211, 1212, 1459, 1460, 1461, 1462, 1463, 1464, 1465, 1466, 1467, 1468, -1
};

static int init_dmap_items_0_70s_list[] =
{
  // dialog control number
  148, 181, 182, 233, 234, 285, 286, 337, 338, 445, 446, 447, 448, 449, 450, 451, 452, 453, 454, 701, 702, 703, 704, 705, 706, 707, 708, 709, 710, 957, 958, 959, 960, 961, 962, 963, 964, 965, 966, 1213, 1214, 1215, 1216, 1217, 1218, 1219, 1220, 1221, 1222, 1469, 1470, 1471, 1472, 1473, 1474, 1475, 1476, 1477, 1478, -1
};

static int init_dmap_items_0_80s_list[] =
{
  // dialog control number
  149, 183, 184, 235, 236, 287, 288, 339, 340, 455, 456, 457, 458, 459, 460, 461, 462, 463, 464, 711, 712, 713, 714, 715, 716, 717, 718, 719, 720, 967, 968, 969, 970, 971, 972, 973, 974, 975, 976, 1223, 1224, 1225, 1226, 1227, 1228, 1229, 1230, 1231, 1232, 1479, 1480, 1481, 1482, 1483, 1484, 1485, 1486, 1487, 1488, -1
};

static int init_dmap_items_0_90s_list[] =
{
  // dialog control number
  150, 185, 186, 237, 238, 289, 290, 341, 342, 465, 466, 467, 468, 469, 470, 471, 472, 473, 474, 721, 722, 723, 724, 725, 726, 727, 728, 729, 730, 977, 978, 979, 980, 981, 982, 983, 984, 985, 986, 1233, 1234, 1235, 1236, 1237, 1238, 1239, 1240, 1241, 1242, 1489, 1490, 1491, 1492, 1493, 1494, 1495, 1496, 1497, 1498, -1
};

static int init_dmap_items_1_00s_list[] =
{
  // dialog control number
  151, 187, 188, 239, 240, 291, 292, 343, 344, 475, 476, 477, 478, 479, 480, 481, 482, 483, 484, 731, 732, 733, 734, 735, 736, 737, 738, 739, 740, 987, 988, 989, 990, 991, 992, 993, 994, 995, 996, 1243, 1244, 1245, 1246, 1247, 1248, 1249, 1250, 1251, 1252, 1499, 1500, 1501, 1502, 1503, 1504, 1505, 1506, 1507, 1508, -1
};

static int init_dmap_items_1_10s_list[] =
{
  // dialog control number
  152, 189, 190, 241, 242, 293, 294, 345, 346, 485, 486, 487, 488, 489, 490, 491, 492, 493, 494, 741, 742, 743, 744, 745, 746, 747, 748, 749, 750, 997, 998, 999, 1000, 1001, 1002, 1003, 1004, 1005, 1006, 1253, 1254, 1255, 1256, 1257, 1258, 1259, 1260, 1261, 1262, 1509, 1510, 1511, 1512, 1513, 1514, 1515, 1516, 1517, 1518, -1
};

static int init_dmap_items_1_20s_list[] =
{
  // dialog control number
  153, 191, 192, 243, 244, 295, 296, 347, 348, 495, 496, 497, 498, 499, 500, 501, 502, 503, 504, 751, 752, 753, 754, 755, 756, 757, 758, 759, 760, 1007, 1008, 1009, 1010, 1011, 1012, 1013, 1014, 1015, 1016, 1263, 1264, 1265, 1266, 1267, 1268, 1269, 1270, 1271, 1272, 1519, 1520, 1521, 1522, 1523, 1524, 1525, 1526, 1527, 1528, -1
};

static int init_dmap_items_1_30s_list[] =
{
  // dialog control number
  154, 193, 194, 245, 246, 297, 298, 349, 350, 505, 506, 507, 508, 509, 510, 511, 512, 513, 514, 761, 762, 763, 764, 765, 766, 767, 768, 769, 770, 1017, 1018, 1019, 1020, 1021, 1022, 1023, 1024, 1025, 1026, 1273, 1274, 1275, 1276, 1277, 1278, 1279, 1280, 1281, 1282, 1529, 1530, 1531, 1532, 1533, 1534, 1535, 1536, 1537, 1538, -1
};

static int init_dmap_items_1_40s_list[] =
{
  // dialog control number
  155, 195, 196, 247, 248, 299, 300, 351, 352, 515, 516, 517, 518, 519, 520, 521, 522, 523, 524, 771, 772, 773, 774, 775, 776, 777, 778, 779, 780, 1027, 1028, 1029, 1030, 1031, 1032, 1033, 1034, 1035, 1036, 1283, 1284, 1285, 1286, 1287, 1288, 1289, 1290, 1291, 1292, 1539, 1540, 1541, 1542, 1543, 1544, 1545, 1546, 1547, 1548, -1
};

static int init_dmap_items_1_50s_list[] =
{
  // dialog control number
  156, 197, 198, 249, 250, 301, 302, 353, 354, 525, 526, 527, 528, 529, 530, 531, 532, 533, 534, 781, 782, 783, 784, 785, 786, 787, 788, 789, 790, 1037, 1038, 1039, 1040, 1041, 1042, 1043, 1044, 1045, 1046, 1293, 1294, 1295, 1296, 1297, 1298, 1299, 1300, 1301, 1302, 1549, 1550, 1551, 1552, 1553, 1554, 1555, 1556, 1557, 1558, -1
};

static int init_dmap_items_1_60s_list[] =
{
  // dialog control number
  157, 199, 200, 251, 252, 303, 304, 355, 356, 535, 536, 537, 538, 539, 540, 541, 542, 543, 544, 791, 792, 793, 794, 795, 796, 797, 798, 799, 800, 1047, 1048, 1049, 1050, 1051, 1052, 1053, 1054, 1055, 1056, 1303, 1304, 1305, 1306, 1307, 1308, 1309, 1310, 1311, 1312, 1559, 1560, 1561, 1562, 1563, 1564, 1565, 1566, 1567, 1568, -1
};

static int init_dmap_items_1_70s_list[] =
{
  // dialog control number
  158, 201, 202, 253, 254, 305, 306, 357, 358, 545, 546, 547, 548, 549, 550, 551, 552, 553, 554, 801, 802, 803, 804, 805, 806, 807, 808, 809, 810, 1057, 1058, 1059, 1060, 1061, 1062, 1063, 1064, 1065, 1066, 1313, 1314, 1315, 1316, 1317, 1318, 1319, 1320, 1321, 1322, 1569, 1570, 1571, 1572, 1573, 1574, 1575, 1576, 1577, 1578, -1
};

static int init_dmap_items_1_80s_list[] =
{
  // dialog control number
  159, 203, 204, 255, 256, 307, 308, 359, 360, 555, 556, 557, 558, 559, 560, 561, 562, 563, 564, 811, 812, 813, 814, 815, 816, 817, 818, 819, 820, 1067, 1068, 1069, 1070, 1071, 1072, 1073, 1074, 1075, 1076, 1323, 1324, 1325, 1326, 1327, 1328, 1329, 1330, 1331, 1332, 1579, 1580, 1581, 1582, 1583, 1584, 1585, 1586, 1587, 1588, -1
};

static int init_dmap_items_1_90s_list[] =
{
  // dialog control number
  160, 205, 206, 257, 258, 309, 310, 361, 362, 565, 566, 567, 568, 569, 570, 571, 572, 573, 574, 821, 822, 823, 824, 825, 826, 827, 828, 829, 830, 1077, 1078, 1079, 1080, 1081, 1082, 1083, 1084, 1085, 1086, 1333, 1334, 1335, 1336, 1337, 1338, 1339, 1340, 1341, 1342, 1589, 1590, 1591, 1592, 1593, 1594, 1595, 1596, 1597, 1598, -1
};

static int init_dmap_items_2_00s_list[] =
{
  // dialog control number
  161, 207, 208, 259, 260, 311, 312, 363, 364, 575, 576, 577, 578, 579, 580, 581, 582, 583, 584, 831, 832, 833, 834, 835, 836, 837, 838, 839, 840, 1087, 1088, 1089, 1090, 1091, 1092, 1093, 1094, 1095, 1096, 1343, 1344, 1345, 1346, 1347, 1348, 1349, 1350, 1351, 1352, 1599, 1600, 1601, 1602, 1603, 1604, 1605, 1606, 1607, 1608, -1
};

static int init_dmap_items_2_10s_list[] =
{
  // dialog control number
  162, 209, 210, 261, 262, 313, 314, 365, 366, 585, 586, 587, 588, 589, 590, 591, 592, 593, 594, 841, 842, 843, 844, 845, 846, 847, 848, 849, 850, 1097, 1098, 1099, 1100, 1101, 1102, 1103, 1104, 1105, 1106, 1353, 1354, 1355, 1356, 1357, 1358, 1359, 1360, 1361, 1362, 1609, 1610, 1611, 1612, 1613, 1614, 1615, 1616, 1617, 1618, -1
};

static int init_dmap_items_2_20s_list[] =
{
  // dialog control number
  163, 211, 212, 263, 264, 315, 316, 367, 368, 595, 596, 597, 598, 599, 600, 601, 602, 603, 604, 851, 852, 853, 854, 855, 856, 857, 858, 859, 860, 1107, 1108, 1109, 1110, 1111, 1112, 1113, 1114, 1115, 1116, 1363, 1364, 1365, 1366, 1367, 1368, 1369, 1370, 1371, 1372, 1619, 1620, 1621, 1622, 1623, 1624, 1625, 1626, 1627, 1628, -1
};

static int init_dmap_items_2_30s_list[] =
{
  // dialog control number
  164, 213, 214, 265, 266, 317, 318, 369, 370, 605, 606, 607, 608, 609, 610, 611, 612, 613, 614, 861, 862, 863, 864, 865, 866, 867, 868, 869, 870, 1117, 1118, 1119, 1120, 1121, 1122, 1123, 1124, 1125, 1126, 1373, 1374, 1375, 1376, 1377, 1378, 1379, 1380, 1381, 1382, 1629, 1630, 1631, 1632, 1633, 1634, 1635, 1636, 1637, 1638, -1
};

static int init_dmap_items_2_40s_list[] =
{
  // dialog control number
  165, 215, 216, 267, 268, 319, 320, 371, 372, 615, 616, 617, 618, 619, 620, 621, 622, 623, 624, 871, 872, 873, 874, 875, 876, 877, 878, 879, 880, 1127, 1128, 1129, 1130, 1131, 1132, 1133, 1134, 1135, 1136, 1383, 1384, 1385, 1386, 1387, 1388, 1389, 1390, 1391, 1392, 1639, 1640, 1641, 1642, 1643, 1644, 1645, 1646, 1647, 1648, -1
};

static int init_dmap_items_2_50s_list[] =
{
  // dialog control number
  166, 217, 218, 269, 270, 321, 322, 373, 374, 625, 626, 627, 628, 629, 630, 881, 882, 883, 884, 885, 886, 1137, 1138, 1139, 1140, 1141, 1142, 1393, 1394, 1395, 1396, 1397, 1398, 1649, 1650, 1651, 1652, 1653, 1654, -1
};

static int init_misc_list[] =
{
  // dialog control number
//  1605, 1606, 1607, 1608, 1609, 1610, 1611, 1612, 1613, 1614, 1615, 1616, 1617, 1618, 1619, 1620, 1621, 1622, 1623, 1624, 1625, 1626, 1627, 1628, 1629, 1630, 1631, 1632, 1633, 1634, 1635, 1636, 1637, 1638, 1639, 1640, 1641, -1
  1655, 1656, 1657, 1658, 1659, 1660, 1661, 1662, 1663, 1664, 1665, 1666, 1667, 1668, 1669, 1670, 1671, 1672, 1673, 1674, 1675, 1676, 1677, 1678, 1679, 1680, 1681, 1682, 1683, 1684, 1685, 1686, 1687, 1688, 1689, 1690, 1691, -1
};


static TABPANEL init_dmap_items_hundreds_tabs[] =
{
  // (text)
  { "000",          D_SELECTED, init_dmap_items_000s_list },
  { "100",          0,          init_dmap_items_100s_list },
  { "200",          0,          init_dmap_items_200s_list },
  { NULL }
};


static TABPANEL init_dmap_items_000s_tabs[] =
{
  // (text)
  { "00",          D_SELECTED, init_dmap_items_0_00s_list },
  { "10",          0,          init_dmap_items_0_10s_list },
  { "20",          0,          init_dmap_items_0_20s_list },
  { "30",          0,          init_dmap_items_0_30s_list },
  { "40",          0,          init_dmap_items_0_40s_list },
  { "50",          0,          init_dmap_items_0_50s_list },
  { "60",          0,          init_dmap_items_0_60s_list },
  { "70",          0,          init_dmap_items_0_70s_list },
  { "80",          0,          init_dmap_items_0_80s_list },
  { "90",          0,          init_dmap_items_0_90s_list },
  { NULL }
};

static TABPANEL init_dmap_items_100s_tabs[] =
{
  // (text)
  { "00",          D_SELECTED, init_dmap_items_1_00s_list },
  { "10",          0,          init_dmap_items_1_10s_list },
  { "20",          0,          init_dmap_items_1_20s_list },
  { "30",          0,          init_dmap_items_1_30s_list },
  { "40",          0,          init_dmap_items_1_40s_list },
  { "50",          0,          init_dmap_items_1_50s_list },
  { "60",          0,          init_dmap_items_1_60s_list },
  { "70",          0,          init_dmap_items_1_70s_list },
  { "80",          0,          init_dmap_items_1_80s_list },
  { "90",          0,          init_dmap_items_1_90s_list },
  { NULL }
};

static TABPANEL init_dmap_items_200s_tabs[] =
{
  // (text)
  { "00",          D_SELECTED, init_dmap_items_2_00s_list },
  { "10",          0,          init_dmap_items_2_10s_list },
  { "20",          0,          init_dmap_items_2_20s_list },
  { "30",          0,          init_dmap_items_2_30s_list },
  { "40",          0,          init_dmap_items_2_40s_list },
  { "50",          0,          init_dmap_items_2_50s_list },
  { NULL }
};

static TABPANEL init_tabs[] =
{
  // (text)
  { "Equipment",       D_SELECTED, init_equipment_list },
  { "Items",           0,          init_items_list },
  { "DMap Items",      0,          init_dmap_items_list },
  { "Misc",            0,          init_misc_list },
  { NULL }
};

//int startdmapxy[6] = {188-68,131-93,188-68,111-93,188-68,120-93};

int d_maxbombsedit_proc(int msg,DIALOG *d,int c)
{
  int ret = jwin_edit_proc(msg,d,c);
  if(msg==MSG_DRAW)
  {
    scare_mouse();
	sprintf((char*)((d+6)->dp), "%d", atoi((char*)(d->dp))/4);
    (d+6)->proc(MSG_DRAW,d+6,0);
    unscare_mouse();
  }

  return ret;
}

static DIALOG init_dlg[] =
{
  /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)     (bg)    (key)    (flags)     (d1)           (d2)     (dp) */
  { jwin_win_proc,     13,   14-3-8,   296,  223+3+8,  vc(14),  vc(1),  0,       D_EXIT,          0,             0,       (void *) "Initialization Data" },
  { jwin_button_proc,     200,  212,  61,   21,   vc(14),  vc(1),  27,      D_EXIT,     0,             0,       (void *) "Cancel" },
  { jwin_button_proc,     60,   212,  61,   21,   vc(14),  vc(1),  13,      D_EXIT,     0,             0,       (void *) "OK" },
  { d_keyboard_proc,   0,    0,    0,    0,    0,       0,      0,       0,          KEY_F1,        0,       (void *) onHelp },
  { jwin_tab_proc,        19, 39-3-8, 284,  167+3+8,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) init_tabs, NULL, (void *)init_dlg },
  // 5(Equipment)
  { jwin_frame_proc,    25+2,    45+8,   77-19,   50,       0,       0,    0,       0,   FR_ETCHED,        0,     NULL },
  { jwin_text_proc,     29+2,    42+8,   40,    8,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) " Sword " },
  { jwin_check_proc,    33+2,    52+8,   64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Wooden" },
  { jwin_check_proc,    33+2,    62+8,   64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "White " },
  { jwin_check_proc,    33+2,    72+8,   64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Magic " },
  { jwin_check_proc,    33+2,    82+8,   64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Master" },
  // 11
  { jwin_frame_proc,    25+2,    99+8,   77-22,   40,       0,       0,    0,       0,   FR_ETCHED,        0,     NULL },
  { jwin_text_proc,     29+2,    96+8,   48,    8,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) " Shield " },
  { jwin_check_proc,    33+2,   106+8,   64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Small" },
  { jwin_check_proc,    33+2,   116+8,   64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Magic" },
  { jwin_check_proc,    33+2,   126+8,   64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Mirror" },
  // 16
  { jwin_frame_proc,    25+2,   143+8,   61-15,   40,       0,       0,    0,       0,   FR_ETCHED,        0,     NULL },
  { jwin_text_proc,     29+2,   140+8,   48,    8,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) " Ring " },
  { jwin_check_proc,    33+2,   150+8,   48,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Blue" },
  { jwin_check_proc,    33+2,   160+8,   48,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Red" },
  { jwin_check_proc,    33+2,   170+8,   48,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Gold" },
  // 21
  { jwin_frame_proc,   110+15,    45+8,   85-25,   30,       0,       0,    0,       0,   FR_ETCHED,        0,     NULL },
  { jwin_text_proc,    114+15,    42+8,   64,    8,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) " Bracelet " },
  { jwin_check_proc,   118+15,    52+8,   72,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Level 1" },
  { jwin_check_proc,   118+15,    62+8,   72,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Level 2" },
  // 25
  { jwin_frame_proc,   110+15,    79+8,   85-25,   30,       0,       0,    0,       0,   FR_ETCHED,        0,     NULL },
  { jwin_text_proc,    114+15,    76+8,   48,    8,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) " Amulet " },
  { jwin_check_proc,   118+15,    86+8,   72,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Level 1" },
  { jwin_check_proc,   118+15,    96+8,   72,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Level 2" },
  // 29
  { jwin_frame_proc,   110+15,   113+8,   69-15,   30,       0,       0,    0,       0,   FR_ETCHED,        0,     NULL },
  { jwin_text_proc,    114+15,   110+8,   48,    8,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) " Wallet " },
  { jwin_check_proc,   118+15,   120+8,   56,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Small" },
  { jwin_check_proc,   118+15,   130+8,   56,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Large" },
  // 33
  { jwin_frame_proc,   110+15,   147+8,   69-15,   30,       0,       0,    0,       0,   FR_ETCHED,        0,     NULL },
  { jwin_text_proc,    114+15,   144+8,   24,    8,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) " Bow " },
  { jwin_check_proc,   118+15,   154+8,   56,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Small" },
  { jwin_check_proc,   118+15,   164+8,   56,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Large" },
  // 37
  { jwin_frame_proc,   203+22,    45+8,   93-23,   70,       0,       0,    0,       0,   FR_ETCHED,        0,     NULL },
  { jwin_text_proc,    207+22,    42+8,   40,    8,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) " Other " },
  { jwin_check_proc,   211+22,    52+8,   80,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Raft" },
  { jwin_check_proc,   211+22,    62+8,   80,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Ladder" },
  { jwin_check_proc,   211+22,    72+8,   80,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Book" },
  { jwin_check_proc,   211+22,    82+8,   80,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Magic Key" },
  { jwin_check_proc,   211+22,    92+8,   80,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Flippers" },
  { jwin_check_proc,   211+22,   102+8,   80,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Boots" },
  // 45
  { d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL },
  { d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL },
  { d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL },
  { d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL },
  { d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL },
  { d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL },
  { d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL },
  { d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL },
  { d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL },
  { d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL },
  { d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL },
  { d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL },
  { d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL },
  { d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL },
  { d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL },
  { d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL },
  { d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL },
  { d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL },
  { d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL },
  { d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL },
  // 65 (Items)
  { jwin_frame_proc,    27,    45+8,   64,   40,       0,       0,    0,       0,   FR_ETCHED,        0,     NULL },
  { jwin_text_proc,     31,    42+8,   64,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) " Boomerang " },
  { jwin_check_proc,    35,    52+8,   64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Wooden" },
  { jwin_check_proc,    35,    62+8,   64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Magic" },
  { jwin_check_proc,    35,    72+8,   64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Fire" },
  // 70
  { jwin_frame_proc,    27,    89+8,   58,   40,       0,       0,    0,       0,   FR_ETCHED,        0,     NULL },
  { jwin_text_proc,     31,    86+8,   48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) " Arrow " },
  { jwin_check_proc,    35,    96+8,   64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Wooden" },
  { jwin_check_proc,    35,   106+8,   64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Silver" },
  { jwin_check_proc,    35,   116+8,   64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Golden" },
  // 75
  { jwin_frame_proc,    27,   133+8,   48,   40,       0,       0,    0,       0,   FR_ETCHED,        0,     NULL },
  { jwin_text_proc,     31,   130+8,   48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) " Potion " },
  { jwin_radio_proc,    35,   140+8,   48,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "None" },
  { jwin_radio_proc,    35,   150+8,   48,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Blue" },
  { jwin_radio_proc,    35,   160+8,   48,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Red" },
  // 80
  { jwin_frame_proc,    93,    45+8,   67,   20,       0,       0,    0,       0,   FR_ETCHED,        0,     NULL },
  { jwin_text_proc,     97,    42+8,   48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) " Whistle " },
  { jwin_check_proc,   101,    52+8,   80,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Recorder" },
  // 83
  { jwin_frame_proc,   154+1,    45+8+34,   50,   20,       0,       0,    0,       0,   FR_ETCHED,        0,     NULL },
  { jwin_text_proc,    158+1,    42+8+34,   48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) " Hammer " },
  { jwin_check_proc,   162+1,    52+8+34,   72,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Small" },
  // 86
  { jwin_frame_proc,    93+2,    93+8-24,   57,   30,       0,       0,    0,       0,   FR_ETCHED,        0,     NULL },
  { jwin_text_proc,     97+2,    90+8-24,   48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) " Hookshot " },
  { jwin_check_proc,   101+2,   100+8-24,   56,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Short" },
  { jwin_check_proc,   101+2,   110+8-24,   56,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Long" },
  // 90
  { jwin_frame_proc,   114-4+20+22+10,    45+8,   45,   30,       0,       0,    0,       0,   FR_ETCHED,        0,     NULL },
  { jwin_text_proc,    118-4+20+22+10,    42+8,   48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) " Candle " },
  { jwin_check_proc,   122-4+20+22+10,    52+8,   48,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Blue" },
  { jwin_check_proc,   122-4+20+22+10,    62+8,   48,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Red" },
  // 94
  { jwin_frame_proc,   217-8,    45+8,   77+9,  138-58,       0,       0,    0,       0,   FR_ETCHED,        0,     NULL },
  { jwin_text_proc,    221-8,    42+8,   80,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) " Other " },
  { jwin_check_proc,   225-8,    52+8,   64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Bait  " },
  { jwin_check_proc,   225-8,    62+8,   64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Letter" },
  { jwin_check_proc,   225-8,    72+8,   64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Wand  " },
  { jwin_check_proc,   225-8,    82+8,   64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Lens  " },
  { jwin_check_proc,   225-8,    92+8,   64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Din's Fire" },
  { jwin_check_proc,   225-8,   102+8,   64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Farore's Wind" },
  { jwin_check_proc,   225-8,   112+8,   64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Nayru's Love" },
  // 103
  { jwin_frame_proc,   120-33+17,    108+3,   77+9+34-34,  138-58+11,       0,       0,    0,       0,   FR_ETCHED,        0,     NULL },
  { jwin_text_proc,    120+4-33+17,  108,   80+33,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) " Bombs " },
  // 105
  { jwin_frame_proc,   120+4-33+17,  118+3,   77+9+34-8-34,  40-3,       0,       0,    0,       0,   FR_ETCHED,        0,     NULL },
  { jwin_text_proc,    124+4-33+17,   118,   48+33-8,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) " Normal " },
  { jwin_ctext_proc,    120+12-33+11+17,   128,   48+33-8,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "Start" },
  { jwin_ctext_proc,    120+12-33+40+11+1+17,   128,   48+33-8,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "Max" },
  // 109
  { jwin_edit_proc,    120+12-33+17,  138,   21,   16,       0,       0,    0,       0,           2,        0,     NULL },
  { d_maxbombsedit_proc,    120+12-33+40+1+17,  138,   21,   16,       0,       0,    0,       0,           2,        0,     NULL },
  { jwin_frame_proc,   120+4-33+17,  158+1+3,   77+9+34-8-34,  40-3,       0,       0,    0,       0,   FR_ETCHED,        0,     NULL },
  { jwin_text_proc,    124+4-33+17,   158+1,   48+33-8,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) " Super " },
  { jwin_ctext_proc,    120+12-33+11+17,   168+1,   48+33-8,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "Start" },
  { jwin_ctext_proc,    120+12-33+40+11+1+17,   168+1,   48+33-8,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "Max" },
  // 115
  { jwin_edit_proc,    120+12-33+17,  178+1,   21,   16,       0,       0,    0,       0,           2,        0,     NULL },
  { jwin_edit_proc,    120+12-33+40+1+17,  178+1,   21,   16,       0,       0,    0,       D_DISABLED,           2,        0,     NULL },
  // 117
  { d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL },
  { d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL },
  { d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL },
  { d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL },
  { d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL },
  { d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL },
  { d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL },
  { d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL },
  { d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL },
  { d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL },
  { d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL },
  { d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL },
  { d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL },
  { d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL },
  { d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL },
  { d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL },
  { d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL },
  { d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL },
  { d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL },
  { d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL },
  // 137
  { jwin_tab_proc,        19+6, 39+22-3-8, 284-12,  167-22-6+3+8,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) init_dmap_items_hundreds_tabs, NULL, (void *)init_dlg },
  { jwin_tab_proc,        19+6+6, 39+22+22-3-8, 284-12-12,  167-22-6-22-6+3+8,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) init_dmap_items_000s_tabs, NULL, (void *)init_dlg },
  { jwin_tab_proc,        19+6+6, 39+22+22-3-8, 284-12-12,  167-22-6-22-6+3+8,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) init_dmap_items_100s_tabs, NULL, (void *)init_dlg },
  { jwin_tab_proc,        19+6+6, 39+22+22-3-8, 284-12-12,  167-22-6-22-6+3+8,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) init_dmap_items_200s_tabs, NULL, (void *)init_dlg },

  // 141
  { jwin_vline_proc,      160, 94,  40,   96,       0,       0,    0,       0,           0,        0,     NULL },
  { jwin_vline_proc,      160, 94,  40,   96,       0,       0,    0,       0,           0,        0,     NULL },
  { jwin_vline_proc,      160, 94,  40,   96,       0,       0,    0,       0,           0,        0,     NULL },
  { jwin_vline_proc,      160, 94,  40,   96,       0,       0,    0,       0,           0,        0,     NULL },
  { jwin_vline_proc,      160, 94,  40,   96,       0,       0,    0,       0,           0,        0,     NULL },
  { jwin_vline_proc,      160, 94,  40,   96,       0,       0,    0,       0,           0,        0,     NULL },
  { jwin_vline_proc,      160, 94,  40,   96,       0,       0,    0,       0,           0,        0,     NULL },
  { jwin_vline_proc,      160, 94,  40,   96,       0,       0,    0,       0,           0,        0,     NULL },
  { jwin_vline_proc,      160, 94,  40,   96,       0,       0,    0,       0,           0,        0,     NULL },
  { jwin_vline_proc,      160, 94,  40,   96,       0,       0,    0,       0,           0,        0,     NULL },
  { jwin_vline_proc,      160, 94,  40,   96,       0,       0,    0,       0,           0,        0,     NULL },
  { jwin_vline_proc,      160, 94,  40,   96,       0,       0,    0,       0,           0,        0,     NULL },
  { jwin_vline_proc,      160, 94,  40,   96,       0,       0,    0,       0,           0,        0,     NULL },
  { jwin_vline_proc,      160, 94,  40,   96,       0,       0,    0,       0,           0,        0,     NULL },
  { jwin_vline_proc,      160, 94,  40,   96,       0,       0,    0,       0,           0,        0,     NULL },
  { jwin_vline_proc,      160, 94,  40,   96,       0,       0,    0,       0,           0,        0,     NULL },
  { jwin_vline_proc,      160, 94,  40,   96,       0,       0,    0,       0,           0,        0,     NULL },
  { jwin_vline_proc,      160, 94,  40,   96,       0,       0,    0,       0,           0,        0,     NULL },
  { jwin_vline_proc,      160, 94,  40,   96,       0,       0,    0,       0,           0,        0,     NULL },
  { jwin_vline_proc,      160, 94,  40,   96,       0,       0,    0,       0,           0,        0,     NULL },
  { jwin_vline_proc,      160, 94,  40,   96,       0,       0,    0,       0,           0,        0,     NULL },
  { jwin_vline_proc,      160, 94,  40,   96,       0,       0,    0,       0,           0,        0,     NULL },
  { jwin_vline_proc,      160, 94,  40,   96,       0,       0,    0,       0,           0,        0,     NULL },
  { jwin_vline_proc,      160, 94,  40,   96,       0,       0,    0,       0,           0,        0,     NULL },
  { jwin_vline_proc,      160, 94,  40,   96,       0,       0,    0,       0,           0,        0,     NULL },
  { jwin_vline_proc,      160, 94,  40,   96,       0,       0,    0,       0,           0,        0,     NULL },

  // 167
  { jwin_ctext_proc,      80,  94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "M" },
  { jwin_ctext_proc,      209, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "M" },
  { jwin_ctext_proc,      80,  94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "M" },
  { jwin_ctext_proc,      209, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "M" },
  { jwin_ctext_proc,      80,  94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "M" },
  { jwin_ctext_proc,      209, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "M" },
  { jwin_ctext_proc,      80,  94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "M" },
  { jwin_ctext_proc,      209, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "M" },
  { jwin_ctext_proc,      80,  94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "M" },
  { jwin_ctext_proc,      209, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "M" },
  { jwin_ctext_proc,      80,  94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "M" },
  { jwin_ctext_proc,      209, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "M" },
  { jwin_ctext_proc,      80,  94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "M" },
  { jwin_ctext_proc,      209, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "M" },
  { jwin_ctext_proc,      80,  94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "M" },
  { jwin_ctext_proc,      209, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "M" },
  { jwin_ctext_proc,      80,  94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "M" },
  { jwin_ctext_proc,      209, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "M" },
  { jwin_ctext_proc,      80,  94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "M" },
  { jwin_ctext_proc,      209, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "M" },
  { jwin_ctext_proc,      80,  94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "M" },
  { jwin_ctext_proc,      209, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "M" },
  { jwin_ctext_proc,      80,  94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "M" },
  { jwin_ctext_proc,      209, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "M" },
  { jwin_ctext_proc,      80,  94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "M" },
  { jwin_ctext_proc,      209, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "M" },
  { jwin_ctext_proc,      80,  94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "M" },
  { jwin_ctext_proc,      209, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "M" },
  { jwin_ctext_proc,      80,  94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "M" },
  { jwin_ctext_proc,      209, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "M" },
  { jwin_ctext_proc,      80,  94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "M" },
  { jwin_ctext_proc,      209, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "M" },
  { jwin_ctext_proc,      80,  94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "M" },
  { jwin_ctext_proc,      209, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "M" },
  { jwin_ctext_proc,      80,  94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "M" },
  { jwin_ctext_proc,      209, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "M" },
  { jwin_ctext_proc,      80,  94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "M" },
  { jwin_ctext_proc,      209, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "M" },
  { jwin_ctext_proc,      80,  94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "M" },
  { jwin_ctext_proc,      209, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "M" },
  { jwin_ctext_proc,      80,  94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "M" },
  { jwin_ctext_proc,      209, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "M" },
  { jwin_ctext_proc,      80,  94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "M" },
  { jwin_ctext_proc,      209, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "M" },
  { jwin_ctext_proc,      80,  94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "M" },
  { jwin_ctext_proc,      209, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "M" },
  { jwin_ctext_proc,      80,  94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "M" },
  { jwin_ctext_proc,      209, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "M" },
  { jwin_ctext_proc,      80,  94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "M" },
  { jwin_ctext_proc,      209, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "M" },
  { jwin_ctext_proc,      80,  94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "M" },
  { jwin_ctext_proc,      209, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "M" },

  // 219
  { jwin_ctext_proc,      92,  94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "C" },
  { jwin_ctext_proc,      221, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "C" },
  { jwin_ctext_proc,      92,  94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "C" },
  { jwin_ctext_proc,      221, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "C" },
  { jwin_ctext_proc,      92,  94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "C" },
  { jwin_ctext_proc,      221, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "C" },
  { jwin_ctext_proc,      92,  94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "C" },
  { jwin_ctext_proc,      221, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "C" },
  { jwin_ctext_proc,      92,  94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "C" },
  { jwin_ctext_proc,      221, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "C" },
  { jwin_ctext_proc,      92,  94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "C" },
  { jwin_ctext_proc,      221, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "C" },
  { jwin_ctext_proc,      92,  94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "C" },
  { jwin_ctext_proc,      221, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "C" },
  { jwin_ctext_proc,      92,  94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "C" },
  { jwin_ctext_proc,      221, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "C" },
  { jwin_ctext_proc,      92,  94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "C" },
  { jwin_ctext_proc,      221, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "C" },
  { jwin_ctext_proc,      92,  94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "C" },
  { jwin_ctext_proc,      221, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "C" },
  { jwin_ctext_proc,      92,  94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "C" },
  { jwin_ctext_proc,      221, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "C" },
  { jwin_ctext_proc,      92,  94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "C" },
  { jwin_ctext_proc,      221, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "C" },
  { jwin_ctext_proc,      92,  94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "C" },
  { jwin_ctext_proc,      221, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "C" },
  { jwin_ctext_proc,      92,  94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "C" },
  { jwin_ctext_proc,      221, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "C" },
  { jwin_ctext_proc,      92,  94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "C" },
  { jwin_ctext_proc,      221, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "C" },
  { jwin_ctext_proc,      92,  94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "C" },
  { jwin_ctext_proc,      221, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "C" },
  { jwin_ctext_proc,      92,  94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "C" },
  { jwin_ctext_proc,      221, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "C" },
  { jwin_ctext_proc,      92,  94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "C" },
  { jwin_ctext_proc,      221, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "C" },
  { jwin_ctext_proc,      92,  94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "C" },
  { jwin_ctext_proc,      221, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "C" },
  { jwin_ctext_proc,      92,  94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "C" },
  { jwin_ctext_proc,      221, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "C" },
  { jwin_ctext_proc,      92,  94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "C" },
  { jwin_ctext_proc,      221, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "C" },
  { jwin_ctext_proc,      92,  94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "C" },
  { jwin_ctext_proc,      221, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "C" },
  { jwin_ctext_proc,      92,  94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "C" },
  { jwin_ctext_proc,      221, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "C" },
  { jwin_ctext_proc,      92,  94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "C" },
  { jwin_ctext_proc,      221, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "C" },
  { jwin_ctext_proc,      92,  94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "C" },
  { jwin_ctext_proc,      221, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "C" },
  { jwin_ctext_proc,      92,  94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "C" },
  { jwin_ctext_proc,      221, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "C" },



  // 271
  { jwin_ctext_proc,      105, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "B" },
  { jwin_ctext_proc,      234, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "B" },
  { jwin_ctext_proc,      105, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "B" },
  { jwin_ctext_proc,      234, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "B" },
  { jwin_ctext_proc,      105, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "B" },
  { jwin_ctext_proc,      234, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "B" },
  { jwin_ctext_proc,      105, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "B" },
  { jwin_ctext_proc,      234, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "B" },
  { jwin_ctext_proc,      105, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "B" },
  { jwin_ctext_proc,      234, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "B" },
  { jwin_ctext_proc,      105, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "B" },
  { jwin_ctext_proc,      234, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "B" },
  { jwin_ctext_proc,      105, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "B" },
  { jwin_ctext_proc,      234, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "B" },
  { jwin_ctext_proc,      105, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "B" },
  { jwin_ctext_proc,      234, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "B" },
  { jwin_ctext_proc,      105, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "B" },
  { jwin_ctext_proc,      234, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "B" },
  { jwin_ctext_proc,      105, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "B" },
  { jwin_ctext_proc,      234, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "B" },
  { jwin_ctext_proc,      105, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "B" },
  { jwin_ctext_proc,      234, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "B" },
  { jwin_ctext_proc,      105, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "B" },
  { jwin_ctext_proc,      234, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "B" },
  { jwin_ctext_proc,      105, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "B" },
  { jwin_ctext_proc,      234, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "B" },
  { jwin_ctext_proc,      105, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "B" },
  { jwin_ctext_proc,      234, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "B" },
  { jwin_ctext_proc,      105, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "B" },
  { jwin_ctext_proc,      234, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "B" },
  { jwin_ctext_proc,      105, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "B" },
  { jwin_ctext_proc,      234, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "B" },
  { jwin_ctext_proc,      105, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "B" },
  { jwin_ctext_proc,      234, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "B" },
  { jwin_ctext_proc,      105, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "B" },
  { jwin_ctext_proc,      234, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "B" },
  { jwin_ctext_proc,      105, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "B" },
  { jwin_ctext_proc,      234, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "B" },
  { jwin_ctext_proc,      105, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "B" },
  { jwin_ctext_proc,      234, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "B" },
  { jwin_ctext_proc,      105, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "B" },
  { jwin_ctext_proc,      234, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "B" },
  { jwin_ctext_proc,      105, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "B" },
  { jwin_ctext_proc,      234, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "B" },
  { jwin_ctext_proc,      105, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "B" },
  { jwin_ctext_proc,      234, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "B" },
  { jwin_ctext_proc,      105, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "B" },
  { jwin_ctext_proc,      234, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "B" },
  { jwin_ctext_proc,      105, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "B" },
  { jwin_ctext_proc,      234, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "B" },
  { jwin_ctext_proc,      105, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "B" },
  { jwin_ctext_proc,      234, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "B" },



  // 323
  { jwin_ctext_proc,      124, 94,  48,    9,   vc(0),  vc(11),    0,       D_DISABLED,           0,        0,     (void *) "K" },
  { jwin_ctext_proc,      253, 94,  48,    9,   vc(0),  vc(11),    0,       D_DISABLED,           0,        0,     (void *) "K" },
  { jwin_ctext_proc,      124, 94,  48,    9,   vc(0),  vc(11),    0,       D_DISABLED,           0,        0,     (void *) "K" },
  { jwin_ctext_proc,      253, 94,  48,    9,   vc(0),  vc(11),    0,       D_DISABLED,           0,        0,     (void *) "K" },
  { jwin_ctext_proc,      124, 94,  48,    9,   vc(0),  vc(11),    0,       D_DISABLED,           0,        0,     (void *) "K" },
  { jwin_ctext_proc,      253, 94,  48,    9,   vc(0),  vc(11),    0,       D_DISABLED,           0,        0,     (void *) "K" },
  { jwin_ctext_proc,      124, 94,  48,    9,   vc(0),  vc(11),    0,       D_DISABLED,           0,        0,     (void *) "K" },
  { jwin_ctext_proc,      253, 94,  48,    9,   vc(0),  vc(11),    0,       D_DISABLED,           0,        0,     (void *) "K" },
  { jwin_ctext_proc,      124, 94,  48,    9,   vc(0),  vc(11),    0,       D_DISABLED,           0,        0,     (void *) "K" },
  { jwin_ctext_proc,      253, 94,  48,    9,   vc(0),  vc(11),    0,       D_DISABLED,           0,        0,     (void *) "K" },
  { jwin_ctext_proc,      124, 94,  48,    9,   vc(0),  vc(11),    0,       D_DISABLED,           0,        0,     (void *) "K" },
  { jwin_ctext_proc,      253, 94,  48,    9,   vc(0),  vc(11),    0,       D_DISABLED,           0,        0,     (void *) "K" },
  { jwin_ctext_proc,      124, 94,  48,    9,   vc(0),  vc(11),    0,       D_DISABLED,           0,        0,     (void *) "K" },
  { jwin_ctext_proc,      253, 94,  48,    9,   vc(0),  vc(11),    0,       D_DISABLED,           0,        0,     (void *) "K" },
  { jwin_ctext_proc,      124, 94,  48,    9,   vc(0),  vc(11),    0,       D_DISABLED,           0,        0,     (void *) "K" },
  { jwin_ctext_proc,      253, 94,  48,    9,   vc(0),  vc(11),    0,       D_DISABLED,           0,        0,     (void *) "K" },
  { jwin_ctext_proc,      124, 94,  48,    9,   vc(0),  vc(11),    0,       D_DISABLED,           0,        0,     (void *) "K" },
  { jwin_ctext_proc,      253, 94,  48,    9,   vc(0),  vc(11),    0,       D_DISABLED,           0,        0,     (void *) "K" },
  { jwin_ctext_proc,      124, 94,  48,    9,   vc(0),  vc(11),    0,       D_DISABLED,           0,        0,     (void *) "K" },
  { jwin_ctext_proc,      253, 94,  48,    9,   vc(0),  vc(11),    0,       D_DISABLED,           0,        0,     (void *) "K" },
  { jwin_ctext_proc,      124, 94,  48,    9,   vc(0),  vc(11),    0,       D_DISABLED,           0,        0,     (void *) "K" },
  { jwin_ctext_proc,      253, 94,  48,    9,   vc(0),  vc(11),    0,       D_DISABLED,           0,        0,     (void *) "K" },
  { jwin_ctext_proc,      124, 94,  48,    9,   vc(0),  vc(11),    0,       D_DISABLED,           0,        0,     (void *) "K" },
  { jwin_ctext_proc,      253, 94,  48,    9,   vc(0),  vc(11),    0,       D_DISABLED,           0,        0,     (void *) "K" },
  { jwin_ctext_proc,      124, 94,  48,    9,   vc(0),  vc(11),    0,       D_DISABLED,           0,        0,     (void *) "K" },
  { jwin_ctext_proc,      253, 94,  48,    9,   vc(0),  vc(11),    0,       D_DISABLED,           0,        0,     (void *) "K" },
  { jwin_ctext_proc,      124, 94,  48,    9,   vc(0),  vc(11),    0,       D_DISABLED,           0,        0,     (void *) "K" },
  { jwin_ctext_proc,      253, 94,  48,    9,   vc(0),  vc(11),    0,       D_DISABLED,           0,        0,     (void *) "K" },
  { jwin_ctext_proc,      124, 94,  48,    9,   vc(0),  vc(11),    0,       D_DISABLED,           0,        0,     (void *) "K" },
  { jwin_ctext_proc,      253, 94,  48,    9,   vc(0),  vc(11),    0,       D_DISABLED,           0,        0,     (void *) "K" },
  { jwin_ctext_proc,      124, 94,  48,    9,   vc(0),  vc(11),    0,       D_DISABLED,           0,        0,     (void *) "K" },
  { jwin_ctext_proc,      253, 94,  48,    9,   vc(0),  vc(11),    0,       D_DISABLED,           0,        0,     (void *) "K" },
  { jwin_ctext_proc,      124, 94,  48,    9,   vc(0),  vc(11),    0,       D_DISABLED,           0,        0,     (void *) "K" },
  { jwin_ctext_proc,      253, 94,  48,    9,   vc(0),  vc(11),    0,       D_DISABLED,           0,        0,     (void *) "K" },
  { jwin_ctext_proc,      124, 94,  48,    9,   vc(0),  vc(11),    0,       D_DISABLED,           0,        0,     (void *) "K" },
  { jwin_ctext_proc,      253, 94,  48,    9,   vc(0),  vc(11),    0,       D_DISABLED,           0,        0,     (void *) "K" },
  { jwin_ctext_proc,      124, 94,  48,    9,   vc(0),  vc(11),    0,       D_DISABLED,           0,        0,     (void *) "K" },
  { jwin_ctext_proc,      253, 94,  48,    9,   vc(0),  vc(11),    0,       D_DISABLED,           0,        0,     (void *) "K" },
  { jwin_ctext_proc,      124, 94,  48,    9,   vc(0),  vc(11),    0,       D_DISABLED,           0,        0,     (void *) "K" },
  { jwin_ctext_proc,      253, 94,  48,    9,   vc(0),  vc(11),    0,       D_DISABLED,           0,        0,     (void *) "K" },
  { jwin_ctext_proc,      124, 94,  48,    9,   vc(0),  vc(11),    0,       D_DISABLED,           0,        0,     (void *) "K" },
  { jwin_ctext_proc,      253, 94,  48,    9,   vc(0),  vc(11),    0,       D_DISABLED,           0,        0,     (void *) "K" },
  { jwin_ctext_proc,      124, 94,  48,    9,   vc(0),  vc(11),    0,       D_DISABLED,           0,        0,     (void *) "K" },
  { jwin_ctext_proc,      253, 94,  48,    9,   vc(0),  vc(11),    0,       D_DISABLED,           0,        0,     (void *) "K" },
  { jwin_ctext_proc,      124, 94,  48,    9,   vc(0),  vc(11),    0,       D_DISABLED,           0,        0,     (void *) "K" },
  { jwin_ctext_proc,      253, 94,  48,    9,   vc(0),  vc(11),    0,       D_DISABLED,           0,        0,     (void *) "K" },
  { jwin_ctext_proc,      124, 94,  48,    9,   vc(0),  vc(11),    0,       D_DISABLED,           0,        0,     (void *) "K" },
  { jwin_ctext_proc,      253, 94,  48,    9,   vc(0),  vc(11),    0,       D_DISABLED,           0,        0,     (void *) "K" },
  { jwin_ctext_proc,      124, 94,  48,    9,   vc(0),  vc(11),    0,       D_DISABLED,           0,        0,     (void *) "K" },
  { jwin_ctext_proc,      253, 94,  48,    9,   vc(0),  vc(11),    0,       D_DISABLED,           0,        0,     (void *) "K" },
  { jwin_ctext_proc,      124, 94,  48,    9,   vc(0),  vc(11),    0,       D_DISABLED,           0,        0,     (void *) "K" },
  { jwin_ctext_proc,      253, 94,  48,    9,   vc(0),  vc(11),    0,       D_DISABLED,           0,        0,     (void *) "K" },



  // 375
  { jwin_rtext_proc,      52,  106,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "0" },
  { jwin_rtext_proc,      52,  124,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "1" },
  { jwin_rtext_proc,      52,  142,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "2" },
  { jwin_rtext_proc,      52,  160,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "3" },
  { jwin_rtext_proc,      52,  178,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "4" },
  { jwin_rtext_proc,      181, 106,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "5" },
  { jwin_rtext_proc,      181, 124,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "6" },
  { jwin_rtext_proc,      181, 142,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "7" },
  { jwin_rtext_proc,      181, 160,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "8" },
  { jwin_rtext_proc,      181, 178,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "9" },
  // 385
  { jwin_rtext_proc,      52,  106,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "10" },
  { jwin_rtext_proc,      52,  124,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "11" },
  { jwin_rtext_proc,      52,  142,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "12" },
  { jwin_rtext_proc,      52,  160,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "13" },
  { jwin_rtext_proc,      52,  178,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "14" },
  { jwin_rtext_proc,      181, 106,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "15" },
  { jwin_rtext_proc,      181, 124,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "16" },
  { jwin_rtext_proc,      181, 142,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "17" },
  { jwin_rtext_proc,      181, 160,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "18" },
  { jwin_rtext_proc,      181, 178,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "19" },
  // 395
  { jwin_rtext_proc,      52,  106,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "20" },
  { jwin_rtext_proc,      52,  124,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "21" },
  { jwin_rtext_proc,      52,  142,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "22" },
  { jwin_rtext_proc,      52,  160,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "23" },
  { jwin_rtext_proc,      52,  178,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "24" },
  { jwin_rtext_proc,      181, 106,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "25" },
  { jwin_rtext_proc,      181, 124,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "26" },
  { jwin_rtext_proc,      181, 142,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "27" },
  { jwin_rtext_proc,      181, 160,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "28" },
  { jwin_rtext_proc,      181, 178,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "29" },
  // 405
  { jwin_rtext_proc,      52,  106,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "30" },
  { jwin_rtext_proc,      52,  124,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "31" },
  { jwin_rtext_proc,      52,  142,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "32" },
  { jwin_rtext_proc,      52,  160,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "33" },
  { jwin_rtext_proc,      52,  178,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "34" },
  { jwin_rtext_proc,      181, 106,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "35" },
  { jwin_rtext_proc,      181, 124,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "36" },
  { jwin_rtext_proc,      181, 142,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "37" },
  { jwin_rtext_proc,      181, 160,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "38" },
  { jwin_rtext_proc,      181, 178,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "39" },
  // 415
  { jwin_rtext_proc,      52,  106,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "40" },
  { jwin_rtext_proc,      52,  124,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "41" },
  { jwin_rtext_proc,      52,  142,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "42" },
  { jwin_rtext_proc,      52,  160,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "43" },
  { jwin_rtext_proc,      52,  178,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "44" },
  { jwin_rtext_proc,      181, 106,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "45" },
  { jwin_rtext_proc,      181, 124,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "46" },
  { jwin_rtext_proc,      181, 142,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "47" },
  { jwin_rtext_proc,      181, 160,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "48" },
  { jwin_rtext_proc,      181, 178,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "49" },
  // 425
  { jwin_rtext_proc,      52,  106,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "50" },
  { jwin_rtext_proc,      52,  124,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "51" },
  { jwin_rtext_proc,      52,  142,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "52" },
  { jwin_rtext_proc,      52,  160,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "53" },
  { jwin_rtext_proc,      52,  178,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "54" },
  { jwin_rtext_proc,      181, 106,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "55" },
  { jwin_rtext_proc,      181, 124,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "56" },
  { jwin_rtext_proc,      181, 142,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "57" },
  { jwin_rtext_proc,      181, 160,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "58" },
  { jwin_rtext_proc,      181, 178,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "59" },
  // 435
  { jwin_rtext_proc,      52,  106,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "60" },
  { jwin_rtext_proc,      52,  124,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "61" },
  { jwin_rtext_proc,      52,  142,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "62" },
  { jwin_rtext_proc,      52,  160,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "63" },
  { jwin_rtext_proc,      52,  178,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "64" },
  { jwin_rtext_proc,      181, 106,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "65" },
  { jwin_rtext_proc,      181, 124,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "66" },
  { jwin_rtext_proc,      181, 142,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "67" },
  { jwin_rtext_proc,      181, 160,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "68" },
  { jwin_rtext_proc,      181, 178,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "69" },
  // 445
  { jwin_rtext_proc,      52,  106,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "70" },
  { jwin_rtext_proc,      52,  124,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "71" },
  { jwin_rtext_proc,      52,  142,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "72" },
  { jwin_rtext_proc,      52,  160,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "73" },
  { jwin_rtext_proc,      52,  178,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "74" },
  { jwin_rtext_proc,      181, 106,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "75" },
  { jwin_rtext_proc,      181, 124,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "76" },
  { jwin_rtext_proc,      181, 142,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "77" },
  { jwin_rtext_proc,      181, 160,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "78" },
  { jwin_rtext_proc,      181, 178,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "79" },
  // 455
  { jwin_rtext_proc,      52,  106,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "80" },
  { jwin_rtext_proc,      52,  124,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "81" },
  { jwin_rtext_proc,      52,  142,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "82" },
  { jwin_rtext_proc,      52,  160,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "83" },
  { jwin_rtext_proc,      52,  178,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "84" },
  { jwin_rtext_proc,      181, 106,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "85" },
  { jwin_rtext_proc,      181, 124,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "86" },
  { jwin_rtext_proc,      181, 142,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "87" },
  { jwin_rtext_proc,      181, 160,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "88" },
  { jwin_rtext_proc,      181, 178,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "89" },
  // 465
  { jwin_rtext_proc,      52,  106,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "90" },
  { jwin_rtext_proc,      52,  124,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "91" },
  { jwin_rtext_proc,      52,  142,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "92" },
  { jwin_rtext_proc,      52,  160,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "93" },
  { jwin_rtext_proc,      52,  178,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "94" },
  { jwin_rtext_proc,      181, 106,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "95" },
  { jwin_rtext_proc,      181, 124,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "96" },
  { jwin_rtext_proc,      181, 142,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "97" },
  { jwin_rtext_proc,      181, 160,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "98" },
  { jwin_rtext_proc,      181, 178,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "99" },
  // 475
  { jwin_rtext_proc,      52,  106,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "100" },
  { jwin_rtext_proc,      52,  124,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "101" },
  { jwin_rtext_proc,      52,  142,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "102" },
  { jwin_rtext_proc,      52,  160,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "103" },
  { jwin_rtext_proc,      52,  178,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "104" },
  { jwin_rtext_proc,      181, 106,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "105" },
  { jwin_rtext_proc,      181, 124,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "106" },
  { jwin_rtext_proc,      181, 142,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "107" },
  { jwin_rtext_proc,      181, 160,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "108" },
  { jwin_rtext_proc,      181, 178,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "109" },
  // 485
  { jwin_rtext_proc,      52,  106,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "110" },
  { jwin_rtext_proc,      52,  124,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "111" },
  { jwin_rtext_proc,      52,  142,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "112" },
  { jwin_rtext_proc,      52,  160,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "113" },
  { jwin_rtext_proc,      52,  178,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "114" },
  { jwin_rtext_proc,      181, 106,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "115" },
  { jwin_rtext_proc,      181, 124,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "116" },
  { jwin_rtext_proc,      181, 142,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "117" },
  { jwin_rtext_proc,      181, 160,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "118" },
  { jwin_rtext_proc,      181, 178,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "119" },
  // 495
  { jwin_rtext_proc,      52,  106,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "120" },
  { jwin_rtext_proc,      52,  124,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "121" },
  { jwin_rtext_proc,      52,  142,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "122" },
  { jwin_rtext_proc,      52,  160,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "123" },
  { jwin_rtext_proc,      52,  178,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "124" },
  { jwin_rtext_proc,      181, 106,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "125" },
  { jwin_rtext_proc,      181, 124,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "126" },
  { jwin_rtext_proc,      181, 142,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "127" },
  { jwin_rtext_proc,      181, 160,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "128" },
  { jwin_rtext_proc,      181, 178,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "129" },
  // 505
  { jwin_rtext_proc,      52,  106,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "130" },
  { jwin_rtext_proc,      52,  124,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "131" },
  { jwin_rtext_proc,      52,  142,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "132" },
  { jwin_rtext_proc,      52,  160,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "133" },
  { jwin_rtext_proc,      52,  178,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "134" },
  { jwin_rtext_proc,      181, 106,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "135" },
  { jwin_rtext_proc,      181, 124,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "136" },
  { jwin_rtext_proc,      181, 142,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "137" },
  { jwin_rtext_proc,      181, 160,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "138" },
  { jwin_rtext_proc,      181, 178,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "139" },
  // 515
  { jwin_rtext_proc,      52,  106,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "140" },
  { jwin_rtext_proc,      52,  124,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "141" },
  { jwin_rtext_proc,      52,  142,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "142" },
  { jwin_rtext_proc,      52,  160,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "143" },
  { jwin_rtext_proc,      52,  178,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "144" },
  { jwin_rtext_proc,      181, 106,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "145" },
  { jwin_rtext_proc,      181, 124,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "146" },
  { jwin_rtext_proc,      181, 142,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "147" },
  { jwin_rtext_proc,      181, 160,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "148" },
  { jwin_rtext_proc,      181, 178,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "149" },
  // 525
  { jwin_rtext_proc,      52,  106,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "150" },
  { jwin_rtext_proc,      52,  124,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "151" },
  { jwin_rtext_proc,      52,  142,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "152" },
  { jwin_rtext_proc,      52,  160,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "153" },
  { jwin_rtext_proc,      52,  178,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "154" },
  { jwin_rtext_proc,      181, 106,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "155" },
  { jwin_rtext_proc,      181, 124,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "156" },
  { jwin_rtext_proc,      181, 142,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "157" },
  { jwin_rtext_proc,      181, 160,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "158" },
  { jwin_rtext_proc,      181, 178,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "159" },
  // 535
  { jwin_rtext_proc,      52,  106,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "160" },
  { jwin_rtext_proc,      52,  124,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "161" },
  { jwin_rtext_proc,      52,  142,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "162" },
  { jwin_rtext_proc,      52,  160,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "163" },
  { jwin_rtext_proc,      52,  178,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "164" },
  { jwin_rtext_proc,      181, 106,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "165" },
  { jwin_rtext_proc,      181, 124,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "166" },
  { jwin_rtext_proc,      181, 142,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "167" },
  { jwin_rtext_proc,      181, 160,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "168" },
  { jwin_rtext_proc,      181, 178,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "169" },
  // 545
  { jwin_rtext_proc,      52,  106,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "170" },
  { jwin_rtext_proc,      52,  124,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "171" },
  { jwin_rtext_proc,      52,  142,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "172" },
  { jwin_rtext_proc,      52,  160,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "173" },
  { jwin_rtext_proc,      52,  178,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "174" },
  { jwin_rtext_proc,      181, 106,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "175" },
  { jwin_rtext_proc,      181, 124,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "176" },
  { jwin_rtext_proc,      181, 142,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "177" },
  { jwin_rtext_proc,      181, 160,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "178" },
  { jwin_rtext_proc,      181, 178,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "179" },
  // 555
  { jwin_rtext_proc,      52,  106,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "180" },
  { jwin_rtext_proc,      52,  124,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "181" },
  { jwin_rtext_proc,      52,  142,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "182" },
  { jwin_rtext_proc,      52,  160,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "183" },
  { jwin_rtext_proc,      52,  178,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "184" },
  { jwin_rtext_proc,      181, 106,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "185" },
  { jwin_rtext_proc,      181, 124,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "186" },
  { jwin_rtext_proc,      181, 142,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "187" },
  { jwin_rtext_proc,      181, 160,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "188" },
  { jwin_rtext_proc,      181, 178,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "189" },
  // 565
  { jwin_rtext_proc,      52,  106,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "190" },
  { jwin_rtext_proc,      52,  124,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "191" },
  { jwin_rtext_proc,      52,  142,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "192" },
  { jwin_rtext_proc,      52,  160,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "193" },
  { jwin_rtext_proc,      52,  178,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "194" },
  { jwin_rtext_proc,      181, 106,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "195" },
  { jwin_rtext_proc,      181, 124,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "196" },
  { jwin_rtext_proc,      181, 142,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "197" },
  { jwin_rtext_proc,      181, 160,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "198" },
  { jwin_rtext_proc,      181, 178,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "199" },
  // 575
  { jwin_rtext_proc,      52,  106,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "200" },
  { jwin_rtext_proc,      52,  124,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "201" },
  { jwin_rtext_proc,      52,  142,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "202" },
  { jwin_rtext_proc,      52,  160,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "203" },
  { jwin_rtext_proc,      52,  178,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "204" },
  { jwin_rtext_proc,      181, 106,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "205" },
  { jwin_rtext_proc,      181, 124,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "206" },
  { jwin_rtext_proc,      181, 142,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "207" },
  { jwin_rtext_proc,      181, 160,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "208" },
  { jwin_rtext_proc,      181, 178,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "209" },
  // 585
  { jwin_rtext_proc,      52,  106,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "210" },
  { jwin_rtext_proc,      52,  124,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "211" },
  { jwin_rtext_proc,      52,  142,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "212" },
  { jwin_rtext_proc,      52,  160,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "213" },
  { jwin_rtext_proc,      52,  178,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "214" },
  { jwin_rtext_proc,      181, 106,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "215" },
  { jwin_rtext_proc,      181, 124,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "216" },
  { jwin_rtext_proc,      181, 142,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "217" },
  { jwin_rtext_proc,      181, 160,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "218" },
  { jwin_rtext_proc,      181, 178,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "219" },
  // 595
  { jwin_rtext_proc,      52,  106,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "220" },
  { jwin_rtext_proc,      52,  124,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "221" },
  { jwin_rtext_proc,      52,  142,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "222" },
  { jwin_rtext_proc,      52,  160,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "223" },
  { jwin_rtext_proc,      52,  178,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "224" },
  { jwin_rtext_proc,      181, 106,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "225" },
  { jwin_rtext_proc,      181, 124,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "226" },
  { jwin_rtext_proc,      181, 142,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "227" },
  { jwin_rtext_proc,      181, 160,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "228" },
  { jwin_rtext_proc,      181, 178,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "229" },
  // 605
  { jwin_rtext_proc,      52,  106,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "230" },
  { jwin_rtext_proc,      52,  124,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "231" },
  { jwin_rtext_proc,      52,  142,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "232" },
  { jwin_rtext_proc,      52,  160,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "233" },
  { jwin_rtext_proc,      52,  178,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "234" },
  { jwin_rtext_proc,      181, 106,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "235" },
  { jwin_rtext_proc,      181, 124,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "236" },
  { jwin_rtext_proc,      181, 142,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "237" },
  { jwin_rtext_proc,      181, 160,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "238" },
  { jwin_rtext_proc,      181, 178,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "239" },
  // 615
  { jwin_rtext_proc,      52,  106,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "240" },
  { jwin_rtext_proc,      52,  124,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "241" },
  { jwin_rtext_proc,      52,  142,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "242" },
  { jwin_rtext_proc,      52,  160,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "243" },
  { jwin_rtext_proc,      52,  178,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "244" },
  { jwin_rtext_proc,      181, 106,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "245" },
  { jwin_rtext_proc,      181, 124,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "246" },
  { jwin_rtext_proc,      181, 142,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "247" },
  { jwin_rtext_proc,      181, 160,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "248" },
  { jwin_rtext_proc,      181, 178,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "249" },
  // 625
  { jwin_rtext_proc,      52,  106,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "250" },
  { jwin_rtext_proc,      52,  124,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "251" },
  { jwin_rtext_proc,      52,  142,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "252" },
  { jwin_rtext_proc,      52,  160,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "253" },
  { jwin_rtext_proc,      52,  178,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "254" },
  { jwin_rtext_proc,      181, 106,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "255" },



  // 631
  { jwin_check_proc,      75,  106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      75,  124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      75,  142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      75,  160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      75,  178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      204, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      204, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      204, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      204, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      204, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  // 641
  { jwin_check_proc,      75,  106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      75,  124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      75,  142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      75,  160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      75,  178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      204, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      204, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      204, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      204, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      204, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  // 651
  { jwin_check_proc,      75,  106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      75,  124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      75,  142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      75,  160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      75,  178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      204, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      204, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      204, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      204, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      204, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  // 661
  { jwin_check_proc,      75,  106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      75,  124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      75,  142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      75,  160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      75,  178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      204, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      204, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      204, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      204, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      204, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  // 671
  { jwin_check_proc,      75,  106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      75,  124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      75,  142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      75,  160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      75,  178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      204, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      204, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      204, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      204, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      204, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  // 681
  { jwin_check_proc,      75,  106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      75,  124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      75,  142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      75,  160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      75,  178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      204, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      204, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      204, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      204, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      204, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  // 691
  { jwin_check_proc,      75,  106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      75,  124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      75,  142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      75,  160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      75,  178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      204, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      204, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      204, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      204, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      204, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  // 701
  { jwin_check_proc,      75,  106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      75,  124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      75,  142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      75,  160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      75,  178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      204, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      204, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      204, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      204, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      204, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  // 711
  { jwin_check_proc,      75,  106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      75,  124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      75,  142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      75,  160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      75,  178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      204, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      204, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      204, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      204, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      204, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  // 721
  { jwin_check_proc,      75,  106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      75,  124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      75,  142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      75,  160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      75,  178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      204, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      204, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      204, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      204, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      204, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  // 731
  { jwin_check_proc,      75,  106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      75,  124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      75,  142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      75,  160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      75,  178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      204, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      204, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      204, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      204, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      204, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  // 741
  { jwin_check_proc,      75,  106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      75,  124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      75,  142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      75,  160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      75,  178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      204, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      204, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      204, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      204, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      204, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  // 751
  { jwin_check_proc,      75,  106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      75,  124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      75,  142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      75,  160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      75,  178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      204, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      204, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      204, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      204, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      204, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  // 761
  { jwin_check_proc,      75,  106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      75,  124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      75,  142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      75,  160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      75,  178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      204, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      204, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      204, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      204, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      204, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  // 771
  { jwin_check_proc,      75,  106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      75,  124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      75,  142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      75,  160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      75,  178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      204, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      204, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      204, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      204, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      204, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  // 781
  { jwin_check_proc,      75,  106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      75,  124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      75,  142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      75,  160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      75,  178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      204, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      204, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      204, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      204, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      204, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  // 791
  { jwin_check_proc,      75,  106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      75,  124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      75,  142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      75,  160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      75,  178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      204, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      204, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      204, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      204, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      204, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  // 801
  { jwin_check_proc,      75,  106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      75,  124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      75,  142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      75,  160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      75,  178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      204, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      204, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      204, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      204, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      204, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  // 811
  { jwin_check_proc,      75,  106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      75,  124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      75,  142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      75,  160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      75,  178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      204, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      204, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      204, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      204, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      204, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  // 821
  { jwin_check_proc,      75,  106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      75,  124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      75,  142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      75,  160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      75,  178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      204, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      204, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      204, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      204, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      204, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  // 831
  { jwin_check_proc,      75,  106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      75,  124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      75,  142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      75,  160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      75,  178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      204, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      204, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      204, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      204, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      204, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  // 841
  { jwin_check_proc,      75,  106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      75,  124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      75,  142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      75,  160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      75,  178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      204, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      204, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      204, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      204, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      204, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  // 851
  { jwin_check_proc,      75,  106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      75,  124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      75,  142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      75,  160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      75,  178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      204, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      204, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      204, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      204, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      204, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  // 861
  { jwin_check_proc,      75,  106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      75,  124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      75,  142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      75,  160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      75,  178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      204, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      204, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      204, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      204, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      204, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  // 871
  { jwin_check_proc,      75,  106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      75,  124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      75,  142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      75,  160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      75,  178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      204, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      204, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      204, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      204, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      204, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  // 881
  { jwin_check_proc,      75,  106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      75,  124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      75,  142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      75,  160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      75,  178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      204, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },



  // 887
  { jwin_check_proc,      88,  106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      88,  124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      88,  142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      88,  160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      88,  178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      217, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      217, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      217, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      217, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      217, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  // 897
  { jwin_check_proc,      88,  106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      88,  124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      88,  142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      88,  160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      88,  178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      217, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      217, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      217, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      217, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      217, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  // 907
  { jwin_check_proc,      88,  106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      88,  124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      88,  142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      88,  160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      88,  178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      217, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      217, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      217, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      217, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      217, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  // 917
  { jwin_check_proc,      88,  106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      88,  124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      88,  142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      88,  160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      88,  178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      217, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      217, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      217, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      217, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      217, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  // 927
  { jwin_check_proc,      88,  106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      88,  124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      88,  142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      88,  160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      88,  178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      217, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      217, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      217, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      217, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      217, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  // 937
  { jwin_check_proc,      88,  106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      88,  124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      88,  142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      88,  160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      88,  178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      217, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      217, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      217, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      217, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      217, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  // 947
  { jwin_check_proc,      88,  106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      88,  124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      88,  142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      88,  160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      88,  178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      217, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      217, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      217, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      217, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      217, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  // 957
  { jwin_check_proc,      88,  106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      88,  124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      88,  142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      88,  160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      88,  178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      217, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      217, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      217, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      217, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      217, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  // 967
  { jwin_check_proc,      88,  106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      88,  124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      88,  142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      88,  160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      88,  178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      217, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      217, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      217, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      217, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      217, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  // 977
  { jwin_check_proc,      88,  106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      88,  124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      88,  142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      88,  160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      88,  178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      217, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      217, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      217, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      217, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      217, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  // 987
  { jwin_check_proc,      88,  106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      88,  124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      88,  142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      88,  160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      88,  178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      217, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      217, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      217, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      217, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      217, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  // 997
  { jwin_check_proc,      88,  106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      88,  124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      88,  142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      88,  160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      88,  178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      217, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      217, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      217, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      217, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      217, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  // 1007
  { jwin_check_proc,      88,  106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      88,  124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      88,  142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      88,  160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      88,  178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      217, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      217, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      217, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      217, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      217, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  // 1017
  { jwin_check_proc,      88,  106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      88,  124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      88,  142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      88,  160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      88,  178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      217, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      217, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      217, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      217, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      217, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  // 1027
  { jwin_check_proc,      88,  106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      88,  124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      88,  142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      88,  160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      88,  178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      217, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      217, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      217, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      217, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      217, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  // 1037
  { jwin_check_proc,      88,  106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      88,  124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      88,  142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      88,  160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      88,  178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      217, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      217, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      217, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      217, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      217, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  // 1047
  { jwin_check_proc,      88,  106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      88,  124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      88,  142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      88,  160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      88,  178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      217, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      217, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      217, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      217, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      217, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  // 1057
  { jwin_check_proc,      88,  106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      88,  124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      88,  142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      88,  160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      88,  178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      217, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      217, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      217, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      217, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      217, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  // 1067
  { jwin_check_proc,      88,  106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      88,  124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      88,  142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      88,  160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      88,  178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      217, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      217, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      217, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      217, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      217, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  // 1077
  { jwin_check_proc,      88,  106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      88,  124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      88,  142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      88,  160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      88,  178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      217, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      217, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      217, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      217, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      217, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  // 1087
  { jwin_check_proc,      88,  106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      88,  124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      88,  142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      88,  160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      88,  178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      217, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      217, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      217, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      217, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      217, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  // 1097
  { jwin_check_proc,      88,  106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      88,  124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      88,  142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      88,  160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      88,  178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      217, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      217, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      217, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      217, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      217, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  // 1107
  { jwin_check_proc,      88,  106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      88,  124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      88,  142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      88,  160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      88,  178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      217, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      217, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      217, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      217, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      217, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  // 1117
  { jwin_check_proc,      88,  106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      88,  124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      88,  142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      88,  160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      88,  178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      217, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      217, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      217, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      217, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      217, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  // 1127
  { jwin_check_proc,      88,  106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      88,  124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      88,  142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      88,  160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      88,  178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      217, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      217, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      217, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      217, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      217, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  // 1137
  { jwin_check_proc,      88,  106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      88,  124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      88,  142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      88,  160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      88,  178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      217, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },




  // 1143
  { jwin_check_proc,      101, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      101, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      101, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      101, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      101, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      230, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      230, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      230, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      230, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      230, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  // 1153
  { jwin_check_proc,      101, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      101, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      101, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      101, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      101, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      230, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      230, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      230, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      230, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      230, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  // 1163
  { jwin_check_proc,      101, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      101, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      101, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      101, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      101, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      230, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      230, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      230, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      230, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      230, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  // 1173
  { jwin_check_proc,      101, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      101, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      101, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      101, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      101, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      230, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      230, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      230, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      230, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      230, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  // 1183
  { jwin_check_proc,      101, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      101, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      101, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      101, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      101, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      230, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      230, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      230, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      230, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      230, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  // 1193
  { jwin_check_proc,      101, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      101, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      101, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      101, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      101, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      230, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      230, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      230, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      230, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      230, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  // 1203
  { jwin_check_proc,      101, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      101, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      101, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      101, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      101, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      230, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      230, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      230, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      230, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      230, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  // 1213
  { jwin_check_proc,      101, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      101, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      101, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      101, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      101, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      230, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      230, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      230, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      230, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      230, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  // 1223
  { jwin_check_proc,      101, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      101, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      101, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      101, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      101, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      230, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      230, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      230, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      230, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      230, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  // 1233
  { jwin_check_proc,      101, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      101, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      101, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      101, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      101, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      230, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      230, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      230, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      230, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      230, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  // 1243
  { jwin_check_proc,      101, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      101, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      101, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      101, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      101, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      230, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      230, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      230, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      230, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      230, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  // 1253
  { jwin_check_proc,      101, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      101, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      101, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      101, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      101, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      230, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      230, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      230, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      230, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      230, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  // 1263
  { jwin_check_proc,      101, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      101, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      101, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      101, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      101, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      230, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      230, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      230, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      230, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      230, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  // 1273
  { jwin_check_proc,      101, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      101, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      101, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      101, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      101, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      230, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      230, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      230, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      230, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      230, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  // 1283
  { jwin_check_proc,      101, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      101, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      101, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      101, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      101, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      230, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      230, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      230, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      230, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      230, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  // 1293
  { jwin_check_proc,      101, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      101, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      101, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      101, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      101, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      230, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      230, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      230, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      230, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      230, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  // 1303
  { jwin_check_proc,      101, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      101, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      101, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      101, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      101, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      230, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      230, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      230, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      230, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      230, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  // 1313
  { jwin_check_proc,      101, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      101, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      101, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      101, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      101, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      230, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      230, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      230, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      230, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      230, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  // 1323
  { jwin_check_proc,      101, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      101, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      101, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      101, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      101, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      230, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      230, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      230, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      230, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      230, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  // 1333
  { jwin_check_proc,      101, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      101, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      101, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      101, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      101, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      230, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      230, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      230, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      230, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      230, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  // 1343
  { jwin_check_proc,      101, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      101, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      101, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      101, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      101, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      230, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      230, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      230, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      230, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      230, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  // 1353
  { jwin_check_proc,      101, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      101, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      101, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      101, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      101, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      230, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      230, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      230, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      230, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      230, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  // 1363
  { jwin_check_proc,      101, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      101, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      101, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      101, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      101, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      230, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      230, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      230, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      230, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      230, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  // 1373
  { jwin_check_proc,      101, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      101, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      101, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      101, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      101, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      230, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      230, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      230, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      230, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      230, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  // 1383
  { jwin_check_proc,      101, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      101, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      101, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      101, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      101, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      230, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      230, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      230, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      230, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      230, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  // 1393
  { jwin_check_proc,      101, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      101, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      101, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      101, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      101, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },
  { jwin_check_proc,      230, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL },





  //1399
  { jwin_edit_proc,       114, 102, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       114, 120, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       114, 138, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       114, 156, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       114, 174, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       243, 102, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       243, 120, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       243, 138, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       243, 156, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       243, 174, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  //1409
  { jwin_edit_proc,       114, 102, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       114, 120, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       114, 138, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       114, 156, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       114, 174, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       243, 102, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       243, 120, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       243, 138, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       243, 156, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       243, 174, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  //1419
  { jwin_edit_proc,       114, 102, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       114, 120, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       114, 138, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       114, 156, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       114, 174, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       243, 102, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       243, 120, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       243, 138, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       243, 156, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       243, 174, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  //1429
  { jwin_edit_proc,       114, 102, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       114, 120, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       114, 138, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       114, 156, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       114, 174, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       243, 102, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       243, 120, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       243, 138, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       243, 156, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       243, 174, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  //1439
  { jwin_edit_proc,       114, 102, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       114, 120, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       114, 138, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       114, 156, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       114, 174, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       243, 102, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       243, 120, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       243, 138, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       243, 156, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       243, 174, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  //1449
  { jwin_edit_proc,       114, 102, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       114, 120, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       114, 138, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       114, 156, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       114, 174, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       243, 102, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       243, 120, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       243, 138, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       243, 156, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       243, 174, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  //1459
  { jwin_edit_proc,       114, 102, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       114, 120, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       114, 138, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       114, 156, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       114, 174, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       243, 102, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       243, 120, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       243, 138, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       243, 156, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       243, 174, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  //1469
  { jwin_edit_proc,       114, 102, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       114, 120, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       114, 138, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       114, 156, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       114, 174, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       243, 102, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       243, 120, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       243, 138, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       243, 156, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       243, 174, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  //1479
  { jwin_edit_proc,       114, 102, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       114, 120, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       114, 138, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       114, 156, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       114, 174, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       243, 102, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       243, 120, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       243, 138, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       243, 156, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       243, 174, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  //1489
  { jwin_edit_proc,       114, 102, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       114, 120, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       114, 138, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       114, 156, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       114, 174, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       243, 102, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       243, 120, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       243, 138, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       243, 156, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       243, 174, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  //1499
  { jwin_edit_proc,       114, 102, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       114, 120, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       114, 138, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       114, 156, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       114, 174, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       243, 102, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       243, 120, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       243, 138, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       243, 156, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       243, 174, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  //1509
  { jwin_edit_proc,       114, 102, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       114, 120, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       114, 138, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       114, 156, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       114, 174, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       243, 102, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       243, 120, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       243, 138, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       243, 156, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       243, 174, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  //1519
  { jwin_edit_proc,       114, 102, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       114, 120, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       114, 138, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       114, 156, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       114, 174, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       243, 102, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       243, 120, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       243, 138, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       243, 156, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       243, 174, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  //1529
  { jwin_edit_proc,       114, 102, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       114, 120, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       114, 138, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       114, 156, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       114, 174, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       243, 102, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       243, 120, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       243, 138, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       243, 156, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       243, 174, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  //1539
  { jwin_edit_proc,       114, 102, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       114, 120, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       114, 138, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       114, 156, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       114, 174, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       243, 102, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       243, 120, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       243, 138, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       243, 156, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       243, 174, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  //1549
  { jwin_edit_proc,       114, 102, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       114, 120, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       114, 138, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       114, 156, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       114, 174, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       243, 102, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       243, 120, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       243, 138, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       243, 156, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       243, 174, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  //1559
  { jwin_edit_proc,       114, 102, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       114, 120, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       114, 138, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       114, 156, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       114, 174, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       243, 102, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       243, 120, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       243, 138, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       243, 156, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       243, 174, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  //1569
  { jwin_edit_proc,       114, 102, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       114, 120, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       114, 138, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       114, 156, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       114, 174, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       243, 102, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       243, 120, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       243, 138, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       243, 156, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       243, 174, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  //1579
  { jwin_edit_proc,       114, 102, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       114, 120, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       114, 138, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       114, 156, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       114, 174, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       243, 102, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       243, 120, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       243, 138, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       243, 156, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       243, 174, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  //1589
  { jwin_edit_proc,       114, 102, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       114, 120, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       114, 138, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       114, 156, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       114, 174, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       243, 102, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       243, 120, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       243, 138, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       243, 156, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       243, 174, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  //1599
  { jwin_edit_proc,       114, 102, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       114, 120, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       114, 138, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       114, 156, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       114, 174, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       243, 102, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       243, 120, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       243, 138, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       243, 156, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       243, 174, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  //1609
  { jwin_edit_proc,       114, 102, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       114, 120, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       114, 138, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       114, 156, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       114, 174, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       243, 102, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       243, 120, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       243, 138, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       243, 156, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       243, 174, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  //1619
  { jwin_edit_proc,       114, 102, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       114, 120, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       114, 138, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       114, 156, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       114, 174, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       243, 102, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       243, 120, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       243, 138, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       243, 156, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       243, 174, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  //1629
  { jwin_edit_proc,       114, 102, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       114, 120, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       114, 138, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       114, 156, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       114, 174, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       243, 102, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       243, 120, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       243, 138, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       243, 156, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       243, 174, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  //1639
  { jwin_edit_proc,       114, 102, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       114, 120, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       114, 138, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       114, 156, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       114, 174, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       243, 102, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       243, 120, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       243, 138, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       243, 156, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       243, 174, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  //1649
  { jwin_edit_proc,       114, 102, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       114, 120, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       114, 138, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       114, 156, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       114, 174, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  { jwin_edit_proc,       243, 102, 21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
  //1655
  { jwin_text_proc,       25,  50+4,   128,   8,    vc(15),  vc(1),  0,       0,          0,             0,        (void *) "Starting DMap: " },
  { d_dropdmaplist_proc,   94,  50,   180,  16,   jwin_pal[jcTEXTFG],  jwin_pal[jcTEXTBG],  0,       0,          0,             0,       (void *) dmaplist, NULL, startdmapxy },

  //1657
  { jwin_text_proc,       25,   68+4,  144,   8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "Heart Containers:" },
  { jwin_text_proc,       25,   86+4,  176,   8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "Starting HP (hearts):" },
  { jwin_text_proc,       25,  104+4,  104,   8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "Continue HP:" },
  { jwin_text_proc,       25,  122+4,   48,   8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "Keys:" },
  { jwin_text_proc,       25+68,  122+4,   64,   8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "Rupies:" },

  //1662
  { jwin_edit_proc,      108,   68,   21,   16,    vc(12),  vc(1),  0,       0,          2,             0,       NULL },
  { jwin_edit_proc,      125,   86,   21,   16,    vc(12),  vc(1),  0,       0,          2,             0,       NULL },
  { jwin_edit_proc,       84,  104,   21,   16,    vc(12),  vc(1),  0,       0,          3,             0,       NULL },
  { jwin_edit_proc,       53,  122,   21,   16,    vc(12),  vc(1),  0,       0,          2,             0,       NULL },
  { jwin_edit_proc,       60+68,  122,   21,   16,    vc(12),  vc(1),  0,       0,          3,             0,       NULL },
  { jwin_check_proc,     107,  108,   24+1,   8+1,    vc(14),  vc(1),  0,       0,          1,             0,       (void *) "%" },
  //1668
  { jwin_frame_proc,      30+136-4+2,  93-22,   40,  50,       0,       0,    0,       0,   FR_ETCHED,        0,     NULL },
  { jwin_text_proc,       30+136+2,  115+30-55-22,   40,   8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) " HCP's " },
  { jwin_radio_proc,      34+136+2,  125+30-55-22,   24+1,   8+1,    vc(14),  vc(1),  0,       0,          1,             0,       (void *) "0" },
  { jwin_radio_proc,      34+136+2,  135+30-55-22,   24+1,   8+1,    vc(14),  vc(1),  0,       0,          1,             0,       (void *) "1" },
  { jwin_radio_proc,      34+136+2,  145+30-55-22,   24+1,   8+1,    vc(14),  vc(1),  0,       0,          1,             0,       (void *) "2" },
  { jwin_radio_proc,      34+136+2,  155+30-55-22,   24+1,   8+1,    vc(14),  vc(1),  0,       0,          1,             0,       (void *) "3" },
  //1674
  { jwin_frame_proc,     116-24+136-8+1,  115+30-55+3-22,   40+22,  50,       0,       0,    0,       0,   FR_ETCHED,        0,     NULL },
  { jwin_text_proc,      116-24+136-4+1,  115+30-55-22,    0,   8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) " Triforce " },
  { jwin_check_proc,     116-24+136+1,  125+30-55-22,   24+1,   8+1,    vc(14),  vc(1),  0,       0,          1,             0,       (void *) "1" },
  { jwin_check_proc,     116-24+136+1,  135+30-55-22,   24+1,   8+1,    vc(14),  vc(1),  0,       0,          1,             0,       (void *) "2" },
  { jwin_check_proc,     116-24+136+1,  145+30-55-22,   24+1,   8+1,    vc(14),  vc(1),  0,       0,          1,             0,       (void *) "3" },
  { jwin_check_proc,     116-24+136+1,  155+30-55-22,   24+1,   8+1,    vc(14),  vc(1),  0,       0,          1,             0,       (void *) "4" },
  { jwin_check_proc,     146-24+136+1,  125+30-55-22,   24+1,   8+1,    vc(14),  vc(1),  0,       0,          1,             0,       (void *) "5" },
  { jwin_check_proc,     146-24+136+1,  135+30-55-22,   24+1,   8+1,    vc(14),  vc(1),  0,       0,          1,             0,       (void *) "6" },
  { jwin_check_proc,     146-24+136+1,  145+30-55-22,   24+1,   8+1,    vc(14),  vc(1),  0,       0,          1,             0,       (void *) "7" },
  { jwin_check_proc,     146-24+136+1,  155+30-55-22,   24+1,   8+1,    vc(14),  vc(1),  0,       0,          1,             0,       (void *) "8" },
  //1684
  { jwin_check_proc,     25,   140+2,   16+1,   8+1,    vc(14),  vc(1),  0,       0,          1,             0,       (void *) "Slash" },
  { jwin_frame_proc,   104+58+24,    108+3+20-6,   77+9+34-34-8,  138-78-10,       0,       0,    0,       0,   FR_ETCHED,        0,     NULL },
  { jwin_text_proc,    108+58+24,   108+20-6,   80+33,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) " Magic " },
  { jwin_ctext_proc,    108+11+4+58+24,   118+20-6,   48+33-8,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "Start" },
  { jwin_ctext_proc,    108+11+41+4+58+24,   118+20-6,   48+33-8,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "Max" },
  { jwin_edit_proc,    108+4+58+24,  128+20-6,   21,   16,       0,       0,    0,       0,           2,        0,     NULL },
  { jwin_edit_proc,    108+41+4+58+24,  128+20-6,   21,   16,       0,       0,    0,       0,           2,        0,     NULL },
  { jwin_check_proc,     108+4+58+24,  148+20-6,   24+1,   8+1,    vc(14),  vc(1),  0,       0,          1,             0,       (void *) "Double" },
  //1692
  { d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL },
  { d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL },
  { d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL },
  { d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL },
  { d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL },
  { d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL },
  { d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL },
  { d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL },
  { d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL },
  { d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL },
  { d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL },
  { d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL },
  { d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL },
  { d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL },
  { d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL },
  { d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL },
  { d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL },
  { d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL },
  { d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL },
  { d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL },
  { d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL },
  { d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL },
  { d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL },
  { d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL },
  { d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL },
  { d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL },
  { d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL },
  { d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL },
  { d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL },
  { d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL },
  { d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL },
  { d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL },
  { d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL },
  { d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL },
  { d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL },
  { d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL },
  { d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL },
  { d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL },
  { d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL },
  { d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL },
  { d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL },
  { d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL },
  { d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL },
  { d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL },
  { d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL },
  { d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL },
  { d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL },
  { d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL },
  { d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL },
  { d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL },
  { d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL },
  { d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL },
  { d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL },
  { d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL },
  { d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL },
  { d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL },
  { d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL },
  { d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL },
  { d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL },
  { d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL },
/**/
  //1642
  { NULL }
};


int onInit()
{
  init_dlg[0].dp2=lfont;

  // equipment
  for(int i=0; i<4; i++)
  {
    init_dlg[i+7].flags = zinit.sword&(1<<i) ? D_SELECTED : 0;
  }
  for(int i=0; i<3; i++)
  {
    init_dlg[i+13].flags = zinit.shield&(1<<i) ? D_SELECTED : 0;
  }
  init_dlg[13].flags |= D_DISABLED;
  for(int i=0; i<3; i++)
  {
    init_dlg[i+18].flags = zinit.ring&(1<<i) ? D_SELECTED : 0;
  }
  for(int i=0; i<2; i++)
  {
    init_dlg[i+23].flags = zinit.bracelet&(1<<i) ? D_SELECTED : 0;
  }
  for(int i=0; i<2; i++)
  {
    init_dlg[i+27].flags = zinit.amulet&(1<<i) ? D_SELECTED : 0;
  }
  for(int i=0; i<2; i++)
  {
    init_dlg[i+31].flags = zinit.wallet&(1<<i) ? D_SELECTED : 0;
  }
  for(int i=0; i<2; i++)
  {
    init_dlg[i+35].flags = zinit.bow&(1<<i) ? D_SELECTED : 0;
  }
  for(int i=0; i<1; i++)
  {
    init_dlg[i+39].flags = zinit.raft&(1<<i) ? D_SELECTED : 0;
  }
  for(int i=0; i<1; i++)
  {
    init_dlg[i+40].flags = zinit.ladder&(1<<i) ? D_SELECTED : 0;
  }
  for(int i=0; i<1; i++)
  {
    init_dlg[i+41].flags = zinit.book&(1<<i) ? D_SELECTED : 0;
  }
  for(int i=0; i<1; i++)
  {
    init_dlg[i+42].flags = zinit.key&(1<<i) ? D_SELECTED : 0;
  }
  for(int i=0; i<1; i++)
  {
    init_dlg[i+43].flags = zinit.flippers&(1<<i) ? D_SELECTED : 0;
  }
  for(int i=0; i<1; i++)
  {
    init_dlg[i+44].flags = zinit.boots&(1<<i) ? D_SELECTED : 0;
  }

  // items
  char bombstring[5];
  char maxbombstring[5];
  char sbombstring[5];
  char maxsbombstring[5];
  sprintf(bombstring, "%d", zinit.bombs);
  sprintf(maxbombstring, "%d", zinit.max_bombs);
  sprintf(sbombstring, "%d", zinit.super_bombs);
  sprintf(maxsbombstring, "%d", zinit.max_bombs/4);
  for(int i=0; i<3; i++)
  {
    init_dlg[i+67].flags = zinit.boomerang&(1<<i) ? D_SELECTED : 0;
  }
  for(int i=0; i<3; i++)
  {
    init_dlg[i+72].flags = zinit.arrow&(1<<i) ? D_SELECTED : 0;
  }
  for(int i=0; i<3; i++)
  {
    init_dlg[i+77].flags = (zinit.potion==i) ? D_SELECTED : 0;
  }
  for(int i=0; i<1; i++)
  {
    init_dlg[i+82].flags = zinit.whistle&(1<<i) ? D_SELECTED : 0;
  }
  for(int i=0; i<1; i++)
  {
    init_dlg[i+85].flags = zinit.hammer&(1<<i) ? D_SELECTED : 0;
  }
  for(int i=0; i<2; i++)
  {
    init_dlg[i+88].flags = zinit.hookshot&(1<<i) ? D_SELECTED : 0;
  }
  for(int i=0; i<2; i++)
  {
    init_dlg[i+92].flags = zinit.candle&(1<<i) ? D_SELECTED : 0;
  }
  for(int i=0; i<1; i++)
  {
    init_dlg[i+96].flags = zinit.bait&(1<<i) ? D_SELECTED : 0;
  }
  for(int i=0; i<1; i++)
  {
    init_dlg[i+97].flags = zinit.letter&(1<<i) ? D_SELECTED : 0;
  }
  for(int i=0; i<1; i++)
  {
    init_dlg[i+98].flags = zinit.wand&(1<<i) ? D_SELECTED : 0;
  }
  for(int i=0; i<1; i++)
  {
    init_dlg[i+99].flags = zinit.lens&(1<<i) ? D_SELECTED : 0;
  }
  for(int i=0; i<1; i++)
  {
    init_dlg[i+100].flags = zinit.dins_fire&(1<<i) ? D_SELECTED : 0;
  }
  for(int i=0; i<1; i++)
  {
    init_dlg[i+111].flags = zinit.farores_wind&(1<<i) ? D_SELECTED : 0;
  }
  for(int i=0; i<1; i++)
  {
    init_dlg[i+112].flags = zinit.nayrus_love&(1<<i) ? D_SELECTED : 0;
  }
  init_dlg[109].dp=bombstring;
  init_dlg[110].dp=maxbombstring;
  init_dlg[115].dp=sbombstring;
  init_dlg[116].dp=maxsbombstring;

  // dmap items

  char key_list[256][3];
  for(int i=0; i<256; i++)
  {
    init_dlg[i+631].flags  = get_bit(zinit.map,i) ? D_SELECTED : 0;
    init_dlg[i+887].flags  = get_bit(zinit.compass,i) ? D_SELECTED : 0;
    init_dlg[i+1143].flags = get_bit(zinit.boss_key,i) ? D_SELECTED : 0;
//    sprintf(key_list[i], "%d", zinit.dmap_keys[i]);
    sprintf(key_list[i], "%d", 0);
    init_dlg[i+1399].dp = key_list[i];
  }

  // misc
  dmap_list_size=MAXDMAPS;
  dmap_list_zero=true;
  init_dlg[1656].d1 = zinit.start_dmap;
  char tempbuf[5];
  char hcstring[5];
  char sheartstring[5];
  char cheartstring[5];
  char keystring[5];
  char rupiestring[5];
  char magicstring[5];
  char maxmagicstring[5];

  sprintf(tempbuf, "0");
  sprintf(hcstring, "%d", zinit.hc);
  sprintf(sheartstring, "%d", zinit.start_heart);
  sprintf(cheartstring, "%d", zinit.cont_heart);
  sprintf(keystring, "%d", zinit.keys);
  sprintf(rupiestring, "%d", zinit.rupies);
  sprintf(magicstring, "%d", zinit.magic);
  sprintf(maxmagicstring, "%d", zinit.max_magic);

  init_dlg[1662].dp=hcstring;
  init_dlg[1663].dp=sheartstring;
  init_dlg[1664].dp=cheartstring;
  init_dlg[1665].dp=keystring;
  init_dlg[1666].dp=rupiestring;
  init_dlg[1667].flags = get_bit(zinit.misc,idM_CONTPERCENT) ? D_SELECTED : 0;

  for (int i=0; i<4; i++)
  {
    init_dlg[i+1670].flags=0;
  }
  init_dlg[zinit.hcp+1670].flags=D_SELECTED;

  for(int i=0; i<8; i++)
  {
    init_dlg[1676].flags = get_bit(&zinit.triforce,i) ? D_SELECTED : 0;
  }


  init_dlg[1684].flags = get_bit(zinit.misc,idM_CANSLASH) ? D_SELECTED : 0;
  init_dlg[1689].dp=magicstring;
  init_dlg[1690].dp=maxmagicstring;
  init_dlg[1691].flags = get_bit(zinit.misc,idM_DOUBLEMAGIC) ? D_SELECTED : 0;

  int ret = zc_popup_dialog(init_dlg,1);
  if (ret==2)
  {

/*
    zinit.=0;
    for(int i=0; i<; i++)
    {
      if(init_dlg[i+].flags & D_SELECTED)
      {
        zinit.|=(1<<i);
      }
    }
*/
    saved=false;

    zinit.sword=0;
    for(int i=0; i<4; i++)
    {
      if(init_dlg[i+7].flags & D_SELECTED)
      {
        zinit.sword|=(1<<i);
      }
    }
    zinit.shield=0;
    for(int i=0; i<3; i++)
    {
      if(init_dlg[i+13].flags & D_SELECTED)
      {
        zinit.shield|=(1<<i);
      }
    }
    zinit.ring=0;
    for(int i=0; i<3; i++)
    {
      if(init_dlg[i+18].flags & D_SELECTED)
      {
        zinit.ring|=(1<<i);
      }
    }
    zinit.bracelet=0;
    for(int i=0; i<2; i++)
    {
      if(init_dlg[i+23].flags & D_SELECTED)
      {
        zinit.bracelet|=(1<<i);
      }
    }
    zinit.amulet=0;
    for(int i=0; i<2; i++)
    {
      if(init_dlg[i+27].flags & D_SELECTED)
      {
        zinit.amulet|=(1<<i);
      }
    }
    zinit.wallet=0;
    for(int i=0; i<2; i++)
    {
      if(init_dlg[i+31].flags & D_SELECTED)
      {
        zinit.wallet|=(1<<i);
      }
    }
    zinit.bow=0;
    for(int i=0; i<2; i++)
    {
      if(init_dlg[i+35].flags & D_SELECTED)
      {
        zinit.bow|=(1<<i);
      }
    }
    zinit.raft=0;
    for(int i=0; i<1; i++)
    {
      if(init_dlg[i+39].flags & D_SELECTED)
      {
        zinit.raft|=(1<<i);
      }
    }
    zinit.ladder=0;
    for(int i=0; i<1; i++)
    {
      if(init_dlg[i+40].flags & D_SELECTED)
      {
        zinit.ladder|=(1<<i);
      }
    }
    zinit.book=0;
    for(int i=0; i<1; i++)
    {
      if(init_dlg[i+41].flags & D_SELECTED)
      {
        zinit.book|=(1<<i);
      }
    }
    zinit.key=0;
    for(int i=0; i<1; i++)
    {
      if(init_dlg[i+42].flags & D_SELECTED)
      {
        zinit.key|=(1<<i);
      }
    }
    zinit.flippers=0;
    for(int i=0; i<1; i++)
    {
      if(init_dlg[i+43].flags & D_SELECTED)
      {
        zinit.flippers|=(1<<i);
      }
    }
    zinit.boots=0;
    for(int i=0; i<1; i++)
    {
      if(init_dlg[i+44].flags & D_SELECTED)
      {
        zinit.boots|=(1<<i);
      }
    }


    // items
    zinit.boomerang=0;
    for(int i=0; i<3; i++)
    {
      if(init_dlg[i+67].flags & D_SELECTED)
      {
        zinit.boomerang|=(1<<i);
      }
    }
    zinit.arrow=0;
    for(int i=0; i<3; i++)
    {
      if(init_dlg[i+72].flags & D_SELECTED)
      {
        zinit.arrow|=(1<<i);
      }
    }
    zinit.potion=0;
    for(int i=0; i<3; i++)
    {
      if(init_dlg[i+77].flags & D_SELECTED)
      {
        zinit.potion=i;
      }
    }
    zinit.whistle=0;
    for(int i=0; i<1; i++)
    {
      if(init_dlg[i+82].flags & D_SELECTED)
      {
        zinit.whistle|=(1<<i);
      }
    }
    zinit.hammer=0;
    for(int i=0; i<1; i++)
    {
      if(init_dlg[i+85].flags & D_SELECTED)
      {
        zinit.hammer|=(1<<i);
      }
    }
    zinit.hookshot=0;
    for(int i=0; i<2; i++)
    {
      if(init_dlg[i+88].flags & D_SELECTED)
      {
        zinit.hookshot|=(1<<i);
      }
    }
    zinit.candle=0;
    for(int i=0; i<2; i++)
    {
      if(init_dlg[i+92].flags & D_SELECTED)
      {
        zinit.candle|=(1<<i);
      }
    }
    zinit.bait=0;
    for(int i=0; i<1; i++)
    {
      if(init_dlg[i+96].flags & D_SELECTED)
      {
        zinit.bait|=(1<<i);
      }
    }
    zinit.letter=0;
    for(int i=0; i<1; i++)
    {
      if(init_dlg[i+97].flags & D_SELECTED)
      {
        zinit.letter|=(1<<i);
      }
    }
    zinit.wand=0;
    for(int i=0; i<1; i++)
    {
      if(init_dlg[i+98].flags & D_SELECTED)
      {
        zinit.wand|=(1<<i);
      }
    }
    zinit.lens=0;
    for(int i=0; i<1; i++)
    {
      if(init_dlg[i+99].flags & D_SELECTED)
      {
        zinit.lens|=(1<<i);
      }
    }
    zinit.dins_fire=0;
    for(int i=0; i<1; i++)
    {
      if(init_dlg[i+100].flags & D_SELECTED)
      {
        zinit.dins_fire|=(1<<i);
      }
    }
    zinit.farores_wind=0;
    for(int i=0; i<1; i++)
    {
      if(init_dlg[i+101].flags & D_SELECTED)
      {
        zinit.farores_wind|=(1<<i);
      }
    }
    zinit.nayrus_love=0;
    for(int i=0; i<1; i++)
    {
      if(init_dlg[i+102].flags & D_SELECTED)
      {
        zinit.nayrus_love|=(1<<i);
      }
    }
    zinit.bombs=atoi(bombstring);
    zinit.max_bombs=atoi(maxbombstring);
    zinit.super_bombs=atoi(sbombstring);

   // dmap items
    for(int i=0; i<256; i++)
    {
      set_bit(zinit.map,i,init_dlg[i+631].flags);
      set_bit(zinit.compass,i,init_dlg[i+887].flags);
      set_bit(zinit.boss_key,i,init_dlg[i+1143].flags);
//      zinit.dmap_keys[i]=atoi(key_list[i]);
    }


    // misc
    zinit.start_dmap = init_dlg[1656].d1;
    zinit.hc = atoi(hcstring);
	zinit.start_heart = atoi(sheartstring);
    zinit.cont_heart = atoi(cheartstring);
    zinit.keys = atoi(keystring);
    zinit.rupies = atoi(rupiestring);
    set_bit(zinit.misc,idM_CONTPERCENT,init_dlg[1667].flags);

    zinit.hcp=0;
    for(int i=0; i<4; i++)
    {
      if(init_dlg[i+1670].flags & D_SELECTED)
      {
        zinit.hcp=i;
      }
    }

    // triforce
    for(int i=0; i<8; i++)
    {
      set_bit(&zinit.triforce,i,init_dlg[1676].flags);
    }


     set_bit(zinit.misc,idM_CANSLASH,init_dlg[1684].flags);
     zinit.magic = atoi(magicstring);
     zinit.max_magic = atoi(maxmagicstring);
     set_bit(zinit.misc,idM_DOUBLEMAGIC,init_dlg[1691].flags);
  }
  return D_O_K;
}


void center_zq_init_dialog()
{
  jwin_center_dialog(init_dlg);
}
