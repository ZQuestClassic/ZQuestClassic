//--------------------------------------------------------
//  Zelda Classic
//  by Jeremy Craner, 1999-2000
//
//  zquest.cc
//
//  Main code for the quest editor.
//
//--------------------------------------------------------
/*
  #include "zquest.h"
  #include "zdefs.h"
  */
#include <stdio.h>

#include "zc_alleg.h"
#include "jwin.h"

#define vc(x)  ((x)+224)                                    // offset to 'VGA color' x (row 14)
extern int jwin_pal[jcMAX];

void initPopulate(int i, DIALOG_PROC proc, int x, int y, int w, int h, int fg, int bg, int key, int flags, int d1, int d2,
                  void *dp, void *dp2 = NULL, void *dp3 = NULL);

/*
  extern FONT *lfont;
  extern int jwin_pal[jcMAX];
  extern bool saved;

  extern int dmap_list_size;
  extern bool dmap_list_zero;
  */

extern int d_dummy_proc(int msg,DIALOG *d,int c);
extern int d_dropdmaplist_proc(int msg,DIALOG *d,int c);
extern char *dmaplist(int index, int *list_size);
extern int onHelp();
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
  65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 128, 129, 130, 131, 132, 133, 134, 135, 136, 1692, 1693, -1
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
  1655, 1656, 1657, 1658, 1659, 1660, 1661, 1662, 1663, 1664, 1665, 1666, 1667, 1668, 1669, 1670, 1671, 1672, 1673, 1674, 1675, 1676, 1677, 1678, 1679, 1680, 1681, 1682, 1683, 1684, 1685, 1686, 1687, 1688, 1689, 1690, 1691, 1694, 1695, 1696, 1697, -1
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

TABPANEL init_tabs[] =
{
  // (text)
  { "Equipment",       D_SELECTED, init_equipment_list },
  { "Items",           0,          init_items_list },
  { "Level Items",      0,          init_dmap_items_list },
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

enum { ws_2_frame, ws_3_frame, ws_max };
char *walkstyles[]= { "2-frame", "3-frame" };

char *walkstylelist(int index, int *list_size)
{
  if(index>=0)
  {
    return walkstyles[index];
  }
  *list_size=ws_max;
  return NULL;
}



DIALOG init_dlg[1753];/* =
                        {
                        // (dialog proc)     (x)   (y)   (w)   (h)   (fg)     (bg)    (key)    (flags)     (d1)           (d2)     (dp)
                        //{ jwin_win_proc,     13,   3,   296,  234,  vc(14),  vc(1),  0,       D_EXIT,          0,             0,       (void *) "Initialization Data" },
                        { jwin_button_proc,     200,  212,  61,   21,   vc(14),  vc(1),  27,      D_EXIT,     0,             0,       (void *) "Cancel" },
                        { jwin_button_proc,     60,   212,  61,   21,   vc(14),  vc(1),  13,      D_EXIT,     0,             0,       (void *) "OK" },
                        { d_keyboard_proc,   0,    0,    0,    0,    0,       0,      0,       0,          KEY_F1,        0,       (void *) onHelp },
                        { jwin_tab_proc,        19, 28, 284,  178,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) init_tabs, NULL, (void *)init_dlg },
                        // 5(Equipment)
                        { jwin_frame_proc,    27,    53,   58,   50,       0,       0,    0,       0,   FR_ETCHED,        0,     NULL },
                        { jwin_text_proc,     31,    50,   40,    8,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) " Sword " },
                        { jwin_check_proc,    35,    60,   64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Wooden" },
                        { jwin_check_proc,    35,    70,   64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "White " },
                        { jwin_check_proc,    35,    80,   64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Magic " },
                        { jwin_check_proc,    35,    90,   64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Master" },
                        // 11
                        { jwin_frame_proc,    27,   107,   55,   40,       0,       0,    0,       0,   FR_ETCHED,        0,     NULL },
                        { jwin_text_proc,     31,   104,   48,    8,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) " Shield " },
                        { jwin_check_proc,    35,   114,   64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Small" },
                        { jwin_check_proc,    35,   124,   64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Magic" },
                        { jwin_check_proc,    35,   134,   64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Mirror" },
                        // 16
                        { jwin_frame_proc,    27,   151,   46,   40,       0,       0,    0,       0,   FR_ETCHED,        0,     NULL },
                        { jwin_text_proc,     31,   148,   48,    8,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) " Ring " },
                        { jwin_check_proc,    35,   158,   48,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Blue" },
                        { jwin_check_proc,    35,   168,   48,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Red" },
                        { jwin_check_proc,    35,   178,   48,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Gold" },
                        // 21
                        { jwin_frame_proc,    93,    53,   60,   30,       0,       0,    0,       0,   FR_ETCHED,        0,     NULL },
                        { jwin_text_proc,     97,    50,   64,    8,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) " Bracelet " },
                        { jwin_check_proc,   102,    60,   72,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Level 1" },
                        { jwin_check_proc,   102,    70,   72,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Level 2" },
                        // 25
                        { jwin_frame_proc,   157,    53,   60,   30,       0,       0,    0,       0,   FR_ETCHED,        0,     NULL },
                        { jwin_text_proc,    161,    50,   48,    8,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) " Amulet " },
                        { jwin_check_proc,   165,    60,   72,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Level 1" },
                        { jwin_check_proc,   165,    70,   72,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Level 2" },
                        // 29
                        { jwin_frame_proc,    93,   102,   54,   30,       0,       0,    0,       0,   FR_ETCHED,        0,     NULL },
                        { jwin_text_proc,     97,    99,   48,    8,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) " Wallet " },
                        { jwin_check_proc,   101,   109,   56,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Small" },
                        { jwin_check_proc,   101,   119,   56,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Large" },
                        // 33
                        { jwin_frame_proc,   163,   102,   54,   30,       0,       0,    0,       0,   FR_ETCHED,        0,     NULL },
                        { jwin_text_proc,    167,    99,   24,    8,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) " Bow " },
                        { jwin_check_proc,   171,   109,   56,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Small" },
                        { jwin_check_proc,   171,   119,   56,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Large" },
                        // 37
                        { jwin_frame_proc,   225,    53,   70,   70,       0,       0,    0,       0,   FR_ETCHED,        0,     NULL },
                        { jwin_text_proc,    229,    50,   40,    8,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) " Other " },
                        { jwin_check_proc,   233,    60,   80,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Raft" },
                        { jwin_check_proc,   233,    70,   80,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Ladder" },
                        { jwin_check_proc,   233,    80,   80,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Book" },
                        { jwin_check_proc,   233,    90,   80,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Magic Key" },
                        { jwin_check_proc,   233,   100,   80,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Flippers" },
                        { jwin_check_proc,   233,   110,   80,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Boots" },
                        // 45
                        { jwin_frame_proc,    80,   151,   56,   40,       0,       0,    0,       0,   FR_ETCHED,        0,     NULL },
                        { jwin_text_proc,     84,   148,   24,    8,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) " Quiver " },
                        { jwin_check_proc,    88,   158,   56,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Small" },
                        { jwin_check_proc,    88,   168,   56,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Medium" },
                        { jwin_check_proc,    88,   178,   56,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Large" },
                        // 49
                        { jwin_frame_proc,   142,   151,   58,   40,       0,       0,    0,       0,   FR_ETCHED,        0,     NULL },
                        { jwin_text_proc,    146,   148,   24,    8,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) " Bomb Bag " },
                        { jwin_check_proc,   150,   158,   56,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Small" },
                        { jwin_check_proc,   150,   168,   56,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Medium" },
                        { jwin_check_proc,   150,   178,   56,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Large" },
                        // 53
                        { jwin_frame_proc,   206,   151,   89,   40,       0,       0,    0,       0,   FR_ETCHED,        0,     NULL },
                        { jwin_text_proc,    210,   148,   24,    8,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) " Super Bomb Bag " },
                        { jwin_check_proc,   214,   158,   56,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Small" },
                        { jwin_check_proc,   214,   178,   56,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Medium" },
                        { jwin_check_proc,   214,   168,   56,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Large" },
                        { d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL },
                        { d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL },
                        { d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL },
                        { d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL },
                        { d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL },
                        // 65 (Items)
                        { jwin_frame_proc,    27,    53,   64,   40,       0,       0,    0,       0,   FR_ETCHED,        0,     NULL },
                        { jwin_text_proc,     31,    50,   64,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) " Boomerang " },
                        { jwin_check_proc,    35,    60,   64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Wooden" },
                        { jwin_check_proc,    35,    70,   64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Magic" },
                        { jwin_check_proc,    35,    80,   64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Fire" },
                        // 70
                        { jwin_frame_proc,   163,   138,  131,   40,       0,       0,    0,       0,   FR_ETCHED,        0,     NULL },
                        { jwin_text_proc,    167,   135,  121,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) " Arrow " },
                        { jwin_check_proc,   171,   145,  137,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Wooden" },
                        { jwin_check_proc,   171,   155,  137,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Silver" },
                        { jwin_check_proc,   171,   165,  137,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Golden" },
                        // 75
                        { jwin_frame_proc,    27,    97,   48,   40,       0,       0,    0,       0,   FR_ETCHED,        0,     NULL },
                        { jwin_text_proc,     31,    94,   48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) " Potion " },
                        { jwin_radio_proc,    35,   104,   48,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "None" },
                        { jwin_radio_proc,    35,   114,   48,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Blue" },
                        { jwin_radio_proc,    35,   124,   48,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Red" },
                        // 80
                        { jwin_frame_proc,    93,    53,   67,   20,       0,       0,    0,       0,   FR_ETCHED,        0,     NULL },
                        { jwin_text_proc,     97,    50,   48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) " Whistle " },
                        { jwin_check_proc,   101,    60,   80,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Recorder" },
                        // 83
                        { jwin_frame_proc,   155,    87,   50,   20,       0,       0,    0,       0,   FR_ETCHED,        0,     NULL },
                        { jwin_text_proc,    159,    84,   48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) " Hammer " },
                        { jwin_check_proc,   163,    94,   72,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Small" },
                        // 86
                        { jwin_frame_proc,    95,   77,    57,   30,       0,       0,    0,       0,   FR_ETCHED,        0,     NULL },
                        { jwin_text_proc,     99,   74,    48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) " Hookshot " },
                        { jwin_check_proc,   103,   84,    56,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Short" },
                        { jwin_check_proc,   103,   94,    56,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Long" },
                        // 90
                        { jwin_frame_proc,   162,   53,   45,   30,       0,       0,    0,       0,   FR_ETCHED,        0,     NULL },
                        { jwin_text_proc,    166,   50,   48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) " Candle " },
                        { jwin_check_proc,   170,   60,   48,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Blue" },
                        { jwin_check_proc,   170,   70,   48,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Red" },
                        // 94
                        { jwin_frame_proc,   209,    53,   86,   80,       0,       0,    0,       0,   FR_ETCHED,        0,     NULL },
                        { jwin_text_proc,    213,    50,   80,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) " Other " },
                        { jwin_check_proc,   217,    60,   64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Bait  " },
                        { jwin_check_proc,   217,    70,   64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Letter" },
                        { jwin_check_proc,   217,    80,   64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Wand  " },
                        { jwin_check_proc,   217,    90,   64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Lens  " },
                        { jwin_check_proc,   217,   100,   64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Din's Fire" },
                        { jwin_check_proc,   217,   110,   64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Farore's Wind" },
                        { jwin_check_proc,   217,   120,   64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Nayru's Love" },
                        // 103
                        { jwin_frame_proc,    76,   111,   86,   91,       0,       0,    0,       0,   FR_ETCHED,        0,     NULL },
                        { jwin_text_proc,     80,   108,  113,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) " Bombs " },
                        // 105
                        { jwin_frame_proc,    80,   121,   78,   37,       0,       0,    0,       0,   FR_ETCHED,        0,     NULL },
                        { jwin_text_proc,     84,   118,   73,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) " Normal " },
                        { jwin_ctext_proc,    99,   128,   73,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "Start" },
                        { jwin_ctext_proc,   140,   128,   73,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "Max" },
                        // 109
                        { jwin_edit_proc,     88,  138,   21,   16,       0,       0,    0,       0,           2,        0,     NULL },
                        { d_maxbombsedit_proc, 129, 138,  21,   16,       0,       0,    0,       0,           2,        0,     NULL },
                        { jwin_frame_proc,    80,   161,  78,   37,       0,       0,    0,       0,   FR_ETCHED,        0,     NULL },
                        { jwin_text_proc,     84,   159,  73,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) " Super " },
                        { jwin_ctext_proc,    99,   169,  73,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "Start" },
                        { jwin_ctext_proc,   140,   169,  73,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "Max" },
                        // 115
                        { jwin_edit_proc,     88,  179,   21,   16,       0,       0,    0,       0,           2,        0,     NULL },
                        { jwin_edit_proc,    129,  179,   21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL },
                        // 117
                        { jwin_ctext_proc,   235,   145,   73,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "Start" },
                        { jwin_ctext_proc,   276,   145,   73,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "Max" },
                        { jwin_edit_proc,    224,   155,   21,   16,       0,       0,    0,       0,           2,        0,     NULL },
                        { jwin_edit_proc,    265,   155,   21,   16,       0,       0,    0,       0,           2,        0,     NULL },
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
                        { jwin_tab_proc,        25, 50, 272,  150,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) init_dmap_items_hundreds_tabs, NULL, (void *)init_dlg },
                        { jwin_tab_proc,        31, 72, 260,  128,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) init_dmap_items_000s_tabs, NULL, (void *)init_dlg },
                        { jwin_tab_proc,        31, 72, 260,  128,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) init_dmap_items_100s_tabs, NULL, (void *)init_dlg },
                        { jwin_tab_proc,        31, 72, 260,  128,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) init_dmap_items_200s_tabs, NULL, (void *)init_dlg },

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
                        { jwin_text_proc,        25,  54,   128,   8,    vc(15),  vc(1),  0,       0,          0,             0,        (void *) "Starting DMap: " },
                        { d_dropdmaplist_proc,   94,  50,   180,  16,   jwin_pal[jcTEXTFG],  jwin_pal[jcTEXTBG],  0,       0,          0,             0,       (void *) dmaplist, NULL, startdmapxy },

                        //1657
                        { jwin_text_proc,       25,   72,  144,   8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "Heart Containers:" },
                        { jwin_text_proc,       25,   90,  176,   8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "Starting HP (hearts):" },
                        { jwin_text_proc,       25,  108,  104,   8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "Continue HP:" },
                        { jwin_text_proc,       25,  126,   48,   8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "Keys:" },
                        { jwin_text_proc,       93,  126,   64,   8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "Rupies:" },

                        //1662
                        { jwin_edit_proc,      108,   68,   21,   16,    vc(12),  vc(1),  0,       0,          2,             0,       NULL },
                        { jwin_edit_proc,      125,   86,   21,   16,    vc(12),  vc(1),  0,       0,          2,             0,       NULL },
                        { jwin_edit_proc,       84,  104,   21,   16,    vc(12),  vc(1),  0,       0,          3,             0,       NULL },
                        { jwin_edit_proc,       53,  122,   21,   16,    vc(12),  vc(1),  0,       0,          2,             0,       NULL },
                        { jwin_edit_proc,      128,  122,   21,   16,    vc(12),  vc(1),  0,       0,          3,             0,       NULL },
                        { jwin_check_proc,     107,  108,   25,    9,    vc(14),  vc(1),  0,       0,          1,             0,       (void *) "%" },
                        //1668
                        { jwin_frame_proc,     164,   71,   40,   50,    0,       0,      0,       0,          FR_ETCHED,     0,       NULL },
                        { jwin_text_proc,      168,   68,   40,    8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) " HCP's " },
                        { jwin_radio_proc,     168,   78,   25,    9,    vc(14),  vc(1),  0,       0,          1,             0,       (void *) "0" },
                        { jwin_radio_proc,     168,   88,   25,    9,    vc(14),  vc(1),  0,       0,          1,             0,       (void *) "1" },
                        { jwin_radio_proc,     168,   98,   25,    9,    vc(14),  vc(1),  0,       0,          1,             0,       (void *) "2" },
                        { jwin_radio_proc,     168,  108,   25,    9,    vc(14),  vc(1),  0,       0,          1,             0,       (void *) "3" },
                        //1674
                        { jwin_frame_proc,     221,   71,   62,  50,    0,       0,      0,       0,           FR_ETCHED,     0,       NULL },
                        { jwin_text_proc,      225,   68,    0,   8,    vc(15),  vc(1),  0,       0,           0,             0,       (void *) " Triforce " },
                        { jwin_check_proc,     229,   78,   25,   9,    vc(14),  vc(1),  0,       0,           1,             0,       (void *) "1" },
                        { jwin_check_proc,     229,   88,   25,   9,    vc(14),  vc(1),  0,       0,           1,             0,       (void *) "2" },
                        { jwin_check_proc,     229,   98,   25,   9,    vc(14),  vc(1),  0,       0,           1,             0,       (void *) "3" },
                        { jwin_check_proc,     229,  108,   25,   9,    vc(14),  vc(1),  0,       0,           1,             0,       (void *) "4" },
                        { jwin_check_proc,     259,   78,   25,   9,    vc(14),  vc(1),  0,       0,           1,             0,       (void *) "5" },
                        { jwin_check_proc,     259,   88,   25,   9,    vc(14),  vc(1),  0,       0,           1,             0,       (void *) "6" },
                        { jwin_check_proc,     259,   98,   25,   9,    vc(14),  vc(1),  0,       0,           1,             0,       (void *) "7" },
                        { jwin_check_proc,     259,  108,   25,   9,    vc(14),  vc(1),  0,       0,           1,             0,       (void *) "8" },
                        //1684
                        { jwin_check_proc,      25,  142,   17,   9,    vc(14),  vc(1),  0,       0,           1,             0,       (void *) "Slash" },
                        { jwin_frame_proc,     186,  125,   78,  50,    0,       0,      0,       0,           FR_ETCHED,     0,       NULL },
                        { jwin_text_proc,      190,  122,  113,   9,    vc(0),   vc(11), 0,       0,           0,             0,       (void *) " Magic " },
                        { jwin_ctext_proc,     205,  132,   73,   9,    vc(0),   vc(11), 0,       0,           0,             0,       (void *) "Start" },
                        { jwin_ctext_proc,     246,  132,   73,   9,    vc(0),   vc(11), 0,       0,           0,             0,       (void *) "Max" },
                        { jwin_edit_proc,      194,  142,   21,  16,    0,       0,      0,       0,           2,             0,       NULL },
                        { jwin_edit_proc,      235,  142,   21,  16,    0,       0,      0,       0,           2,             0,       NULL },
                        { jwin_check_proc,     194,  162,   25,   9,    vc(14),  vc(1),  0,       0,           1,             0,       (void *) "Double" },
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

                        //1752
                        { NULL }
                        };*/

void initPopulate(int i, DIALOG_PROC proc, int x, int y, int w, int h, int fg, int bg, int key, int flags, int d1, int d2,
                  void *dp, void *dp2, void *dp3)
{
  init_dlg[i].proc = proc;
  init_dlg[i].x = x;
  init_dlg[i].y = y;
  init_dlg[i].w = w;
  init_dlg[i].h = h;
  init_dlg[i].fg = fg;
  init_dlg[i].bg = bg;
  init_dlg[i].key = key;
  init_dlg[i].flags = flags;
  init_dlg[i].d1 = d1;
  init_dlg[i].d2 = d2;
  init_dlg[i].dp = dp;
  init_dlg[i].dp2 = dp2;
  init_dlg[i].dp3 = dp3;
}

void PopulateInitDialog()
{
  int i=0;
  /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)     (bg)    (key)    (flags)     (d1)           (d2)     (dp) */
  initPopulate(i, jwin_win_proc,     13,   3,   296,  234,  vc(14),  vc(1),  0,       D_EXIT,          0,             0,       (void *) "Initialization Data" ); i++;
  initPopulate(i, jwin_button_proc,     200,  212,  61,   21,   vc(14),  vc(1),  27,      D_EXIT,     0,             0,       (void *) "Cancel" ); i++;
  initPopulate(i, jwin_button_proc,     60,   212,  61,   21,   vc(14),  vc(1),  13,      D_EXIT,     0,             0,       (void *) "OK" ); i++;
  initPopulate(i, d_keyboard_proc,   0,    0,    0,    0,    0,       0,      0,       0,          KEY_F1,        0,       (void *) onHelp ); i++;
  initPopulate(i, jwin_tab_proc,        19, 28, 284,  178,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) init_tabs, NULL, (void *)init_dlg ); i++;
  // 5(Equipment)
  initPopulate(i, jwin_frame_proc,    27,    53,   58,   50,       0,       0,    0,       0,   FR_ETCHED,        0,     NULL ); i++;
  initPopulate(i, jwin_text_proc,     31,    50,   40,    8,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) " Sword " ); i++;
  initPopulate(i, jwin_check_proc,    35,    60,   64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Wooden" ); i++;
  initPopulate(i, jwin_check_proc,    35,    70,   64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "White " ); i++;
  initPopulate(i, jwin_check_proc,    35,    80,   64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Magic " ); i++;
  initPopulate(i, jwin_check_proc,    35,    90,   64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Master" ); i++;
  // 11
  initPopulate(i, jwin_frame_proc,    27,   107,   55,   40,       0,       0,    0,       0,   FR_ETCHED,        0,     NULL ); i++;
  initPopulate(i, jwin_text_proc,     31,   104,   48,    8,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) " Shield " ); i++;
  initPopulate(i, jwin_check_proc,    35,   114,   64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Small" ); i++;
  initPopulate(i, jwin_check_proc,    35,   124,   64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Magic" ); i++;
  initPopulate(i, jwin_check_proc,    35,   134,   64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Mirror" ); i++;
  // 16
  initPopulate(i, jwin_frame_proc,    27,   151,   46,   40,       0,       0,    0,       0,   FR_ETCHED,        0,     NULL ); i++;
  initPopulate(i, jwin_text_proc,     31,   148,   48,    8,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) " Ring " ); i++;
  initPopulate(i, jwin_check_proc,    35,   158,   48,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Blue" ); i++;
  initPopulate(i, jwin_check_proc,    35,   168,   48,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Red" ); i++;
  initPopulate(i, jwin_check_proc,    35,   178,   48,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Gold" ); i++;
  // 21
  initPopulate(i, jwin_frame_proc,    93,    53,   60,   30,       0,       0,    0,       0,   FR_ETCHED,        0,     NULL ); i++;
  initPopulate(i, jwin_text_proc,     97,    50,   64,    8,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) " Bracelet " ); i++;
  initPopulate(i, jwin_check_proc,   102,    60,   72,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Level 1" ); i++;
  initPopulate(i, jwin_check_proc,   102,    70,   72,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Level 2" ); i++;
  // 25
  initPopulate(i, jwin_frame_proc,   157,    53,   60,   30,       0,       0,    0,       0,   FR_ETCHED,        0,     NULL ); i++;
  initPopulate(i, jwin_text_proc,    161,    50,   48,    8,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) " Amulet " ); i++;
  initPopulate(i, jwin_check_proc,   165,    60,   72,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Level 1" ); i++;
  initPopulate(i, jwin_check_proc,   165,    70,   72,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Level 2" ); i++;
  // 29
  initPopulate(i, jwin_frame_proc,    93,   102,   54,   30,       0,       0,    0,       0,   FR_ETCHED,        0,     NULL ); i++;
  initPopulate(i, jwin_text_proc,     97,    99,   48,    8,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) " Wallet " ); i++;
  initPopulate(i, jwin_check_proc,   101,   109,   56,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Small" ); i++;
  initPopulate(i, jwin_check_proc,   101,   119,   56,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Large" ); i++;
  // 33
  initPopulate(i, jwin_frame_proc,   163,   102,   54,   30,       0,       0,    0,       0,   FR_ETCHED,        0,     NULL ); i++;
  initPopulate(i, jwin_text_proc,    167,    99,   24,    8,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) " Bow " ); i++;
  initPopulate(i, jwin_check_proc,   171,   109,   56,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Small" ); i++;
  initPopulate(i, jwin_check_proc,   171,   119,   56,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Large" ); i++;
  // 37
  initPopulate(i, jwin_frame_proc,   225,    53,   70,   70,       0,       0,    0,       0,   FR_ETCHED,        0,     NULL ); i++;
  initPopulate(i, jwin_text_proc,    229,    50,   40,    8,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) " Other " ); i++;
  initPopulate(i, jwin_check_proc,   233,    60,   80,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Raft" ); i++;
  initPopulate(i, jwin_check_proc,   233,    70,   80,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Ladder" ); i++;
  initPopulate(i, jwin_check_proc,   233,    80,   80,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Book" ); i++;
  initPopulate(i, jwin_check_proc,   233,    90,   80,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Magic Key" ); i++;
  initPopulate(i, jwin_check_proc,   233,   100,   80,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Flippers" ); i++;
  initPopulate(i, jwin_check_proc,   233,   110,   80,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Boots" ); i++;
  // 45
  initPopulate(i, jwin_frame_proc,    80,   151,   56,   40,       0,       0,    0,       0,   FR_ETCHED,        0,     NULL ); i++;
  initPopulate(i, jwin_text_proc,     84,   148,   24,    8,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) " Quiver " ); i++;
  initPopulate(i, jwin_check_proc,    88,   158,   56,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Small" ); i++;
  initPopulate(i, jwin_check_proc,    88,   168,   56,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Medium" ); i++;
  initPopulate(i, jwin_check_proc,    88,   178,   56,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Large" ); i++;
  // 49
  initPopulate(i, jwin_frame_proc,   142,   151,   58,   40,       0,       0,    0,       0,   FR_ETCHED,        0,     NULL ); i++;
  initPopulate(i, jwin_text_proc,    146,   148,   24,    8,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) " Bomb Bag " ); i++;
  initPopulate(i, jwin_check_proc,   150,   158,   56,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Small" ); i++;
  initPopulate(i, jwin_check_proc,   150,   168,   56,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Medium" ); i++;
  initPopulate(i, jwin_check_proc,   150,   178,   56,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Large" ); i++;
  // 53
  initPopulate(i, jwin_frame_proc,   206,   151,   89,   40,       0,       0,    0,       0,   FR_ETCHED,        0,     NULL ); i++;
  initPopulate(i, jwin_text_proc,    210,   148,   24,    8,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) " Super Bomb Bag " ); i++;
  initPopulate(i, jwin_check_proc,   214,   158,   56,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Small" ); i++;
  initPopulate(i, jwin_check_proc,   214,   168,   56,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Medium" ); i++;
  initPopulate(i, jwin_check_proc,   214,   178,   56,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Large" ); i++;
  initPopulate(i, d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL ); i++;
  initPopulate(i, d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL ); i++;
  initPopulate(i, d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL ); i++;
  initPopulate(i, d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL ); i++;
  initPopulate(i, d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL ); i++;
  // 65 (Items)
  initPopulate(i, jwin_frame_proc,    27,    53,   64,   40,       0,       0,    0,       0,   FR_ETCHED,        0,     NULL ); i++;
  initPopulate(i, jwin_text_proc,     31,    50,   64,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) " Boomerang " ); i++;
  initPopulate(i, jwin_check_proc,    35,    60,   64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Wooden" ); i++;
  initPopulate(i, jwin_check_proc,    35,    70,   64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Magic" ); i++;
  initPopulate(i, jwin_check_proc,    35,    80,   64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Fire" ); i++;
  // 70
  initPopulate(i, jwin_frame_proc,   163,   138,  131,   40,       0,       0,    0,       0,   FR_ETCHED,        0,     NULL ); i++;
  initPopulate(i, jwin_text_proc,    167,   135,  121,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) " Arrow " ); i++;
  initPopulate(i, jwin_check_proc,   171,   145,  137,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Wooden" ); i++;
  initPopulate(i, jwin_check_proc,   171,   155,  137,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Silver" ); i++;
  initPopulate(i, jwin_check_proc,   171,   165,  137,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Golden" ); i++;
  // 75
  initPopulate(i, jwin_frame_proc,    27,    97,   48,   40,       0,       0,    0,       0,   FR_ETCHED,        0,     NULL ); i++;
  initPopulate(i, jwin_text_proc,     31,    94,   48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) " Potion " ); i++;
  initPopulate(i, jwin_radio_proc,    35,   104,   48,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "None" ); i++;
  initPopulate(i, jwin_radio_proc,    35,   114,   48,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Blue" ); i++;
  initPopulate(i, jwin_radio_proc,    35,   124,   48,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Red" ); i++;
  // 80
  initPopulate(i, jwin_frame_proc,    93,    53,   67,   20,       0,       0,    0,       0,   FR_ETCHED,        0,     NULL ); i++;
  initPopulate(i, jwin_text_proc,     97,    50,   48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) " Whistle " ); i++;
  initPopulate(i, jwin_check_proc,   101,    60,   80,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Recorder" ); i++;
  // 83
  initPopulate(i, jwin_frame_proc,   155,    87,   50,   20,       0,       0,    0,       0,   FR_ETCHED,        0,     NULL ); i++;
  initPopulate(i, jwin_text_proc,    159,    84,   48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) " Hammer " ); i++;
  initPopulate(i, jwin_check_proc,   163,    94,   72,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Small" ); i++;
  // 86
  initPopulate(i, jwin_frame_proc,    95,   77,    57,   30,       0,       0,    0,       0,   FR_ETCHED,        0,     NULL ); i++;
  initPopulate(i, jwin_text_proc,     99,   74,    48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) " Hookshot " ); i++;
  initPopulate(i, jwin_check_proc,   103,   84,    56,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Short" ); i++;
  initPopulate(i, jwin_check_proc,   103,   94,    56,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Long" ); i++;
  // 90
  initPopulate(i, jwin_frame_proc,   162,   53,   45,   30,       0,       0,    0,       0,   FR_ETCHED,        0,     NULL ); i++;
  initPopulate(i, jwin_text_proc,    166,   50,   48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) " Candle " ); i++;
  initPopulate(i, jwin_check_proc,   170,   60,   48,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Blue" ); i++;
  initPopulate(i, jwin_check_proc,   170,   70,   48,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Red" ); i++;
  // 94
  initPopulate(i, jwin_frame_proc,   209,    53,   86,   80,       0,       0,    0,       0,   FR_ETCHED,        0,     NULL ); i++;
  initPopulate(i, jwin_text_proc,    213,    50,   80,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) " Other " ); i++;
  initPopulate(i, jwin_check_proc,   217,    60,   64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Bait  " ); i++;
  initPopulate(i, jwin_check_proc,   217,    70,   64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Letter" ); i++;
  initPopulate(i, jwin_check_proc,   217,    80,   64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Wand  " ); i++;
  initPopulate(i, jwin_check_proc,   217,    90,   64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Lens  " ); i++;
  initPopulate(i, jwin_check_proc,   217,   100,   64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Din's Fire" ); i++;
  initPopulate(i, jwin_check_proc,   217,   110,   64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Farore's Wind" ); i++;
  initPopulate(i, jwin_check_proc,   217,   120,   64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Nayru's Love" ); i++;
  // 103
  initPopulate(i, jwin_frame_proc,    76,   111,   86,   91,       0,       0,    0,       0,   FR_ETCHED,        0,     NULL ); i++;
  initPopulate(i, jwin_text_proc,     80,   108,  113,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) " Bombs " ); i++;
  // 105
  initPopulate(i, jwin_frame_proc,    80,   121,   78,   37,       0,       0,    0,       0,   FR_ETCHED,        0,     NULL ); i++;
  initPopulate(i, jwin_text_proc,     84,   118,   73,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) " Normal " ); i++;
  initPopulate(i, jwin_ctext_proc,    99,   128,   73,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "Start" ); i++;
  initPopulate(i, jwin_ctext_proc,   140,   128,   73,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "Max" ); i++;
  // 109
  initPopulate(i, jwin_edit_proc,     88,  138,   21,   16,       0,       0,    0,       0,           2,        0,     NULL ); i++;
  initPopulate(i, d_maxbombsedit_proc, 129, 138,  21,   16,       0,       0,    0,       0,           2,        0,     NULL ); i++;
  initPopulate(i, jwin_frame_proc,    80,   161,  78,   37,       0,       0,    0,       0,   FR_ETCHED,        0,     NULL ); i++;
  initPopulate(i, jwin_text_proc,     84,   159,  73,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) " Super " ); i++;
  initPopulate(i, jwin_ctext_proc,    99,   169,  73,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "Start" ); i++;
  initPopulate(i, jwin_ctext_proc,   140,   169,  73,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "Max" ); i++;
  // 115
  initPopulate(i, jwin_edit_proc,     88,  179,   21,   16,       0,       0,    0,       0,           2,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,    129,  179,   21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL ); i++;
  // 117
  initPopulate(i, jwin_ctext_proc,   235,   145,   73,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "Start" ); i++;
  initPopulate(i, jwin_ctext_proc,   276,   145,   73,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "Max" ); i++;
  initPopulate(i, jwin_edit_proc,    224,   155,   21,   16,       0,       0,    0,       0,           2,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,    265,   155,   21,   16,       0,       0,    0,       0,           2,        0,     NULL ); i++;
  initPopulate(i, d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL ); i++;
  initPopulate(i, d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL ); i++;
  initPopulate(i, d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL ); i++;
  initPopulate(i, d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL ); i++;
  initPopulate(i, d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL ); i++;
  initPopulate(i, d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL ); i++;
  initPopulate(i, d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL ); i++;
  initPopulate(i, d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL ); i++;
  initPopulate(i, d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL ); i++;
  initPopulate(i, d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL ); i++;
  initPopulate(i, d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL ); i++;
  initPopulate(i, d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL ); i++;
  initPopulate(i, d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL ); i++;
  initPopulate(i, d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL ); i++;
  initPopulate(i, d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL ); i++;
  initPopulate(i, d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL ); i++;

  // 137
  initPopulate(i, jwin_tab_proc,        25, 50, 272,  150,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) init_dmap_items_hundreds_tabs, NULL, (void *)init_dlg ); i++;
  initPopulate(i, jwin_tab_proc,        31, 72, 260,  128,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) init_dmap_items_000s_tabs, NULL, (void *)init_dlg ); i++;
  initPopulate(i, jwin_tab_proc,        31, 72, 260,  128,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) init_dmap_items_100s_tabs, NULL, (void *)init_dlg ); i++;
  initPopulate(i, jwin_tab_proc,        31, 72, 260,  128,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) init_dmap_items_200s_tabs, NULL, (void *)init_dlg ); i++;

  // 141
  initPopulate(i, jwin_vline_proc,      160, 94,  40,   96,       0,       0,    0,       0,           0,        0,     NULL ); i++;
  initPopulate(i, jwin_vline_proc,      160, 94,  40,   96,       0,       0,    0,       0,           0,        0,     NULL ); i++;
  initPopulate(i, jwin_vline_proc,      160, 94,  40,   96,       0,       0,    0,       0,           0,        0,     NULL ); i++;
  initPopulate(i, jwin_vline_proc,      160, 94,  40,   96,       0,       0,    0,       0,           0,        0,     NULL ); i++;
  initPopulate(i, jwin_vline_proc,      160, 94,  40,   96,       0,       0,    0,       0,           0,        0,     NULL ); i++;
  initPopulate(i, jwin_vline_proc,      160, 94,  40,   96,       0,       0,    0,       0,           0,        0,     NULL ); i++;
  initPopulate(i, jwin_vline_proc,      160, 94,  40,   96,       0,       0,    0,       0,           0,        0,     NULL ); i++;
  initPopulate(i, jwin_vline_proc,      160, 94,  40,   96,       0,       0,    0,       0,           0,        0,     NULL ); i++;
  initPopulate(i, jwin_vline_proc,      160, 94,  40,   96,       0,       0,    0,       0,           0,        0,     NULL ); i++;
  initPopulate(i, jwin_vline_proc,      160, 94,  40,   96,       0,       0,    0,       0,           0,        0,     NULL ); i++;
  initPopulate(i, jwin_vline_proc,      160, 94,  40,   96,       0,       0,    0,       0,           0,        0,     NULL ); i++;
  initPopulate(i, jwin_vline_proc,      160, 94,  40,   96,       0,       0,    0,       0,           0,        0,     NULL ); i++;
  initPopulate(i, jwin_vline_proc,      160, 94,  40,   96,       0,       0,    0,       0,           0,        0,     NULL ); i++;
  initPopulate(i, jwin_vline_proc,      160, 94,  40,   96,       0,       0,    0,       0,           0,        0,     NULL ); i++;
  initPopulate(i, jwin_vline_proc,      160, 94,  40,   96,       0,       0,    0,       0,           0,        0,     NULL ); i++;
  initPopulate(i, jwin_vline_proc,      160, 94,  40,   96,       0,       0,    0,       0,           0,        0,     NULL ); i++;
  initPopulate(i, jwin_vline_proc,      160, 94,  40,   96,       0,       0,    0,       0,           0,        0,     NULL ); i++;
  initPopulate(i, jwin_vline_proc,      160, 94,  40,   96,       0,       0,    0,       0,           0,        0,     NULL ); i++;
  initPopulate(i, jwin_vline_proc,      160, 94,  40,   96,       0,       0,    0,       0,           0,        0,     NULL ); i++;
  initPopulate(i, jwin_vline_proc,      160, 94,  40,   96,       0,       0,    0,       0,           0,        0,     NULL ); i++;
  initPopulate(i, jwin_vline_proc,      160, 94,  40,   96,       0,       0,    0,       0,           0,        0,     NULL ); i++;
  initPopulate(i, jwin_vline_proc,      160, 94,  40,   96,       0,       0,    0,       0,           0,        0,     NULL ); i++;
  initPopulate(i, jwin_vline_proc,      160, 94,  40,   96,       0,       0,    0,       0,           0,        0,     NULL ); i++;
  initPopulate(i, jwin_vline_proc,      160, 94,  40,   96,       0,       0,    0,       0,           0,        0,     NULL ); i++;
  initPopulate(i, jwin_vline_proc,      160, 94,  40,   96,       0,       0,    0,       0,           0,        0,     NULL ); i++;
  initPopulate(i, jwin_vline_proc,      160, 94,  40,   96,       0,       0,    0,       0,           0,        0,     NULL ); i++;

  // 167
  initPopulate(i, jwin_ctext_proc,      80,  94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "M" ); i++;
  initPopulate(i, jwin_ctext_proc,      209, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "M" ); i++;
  initPopulate(i, jwin_ctext_proc,      80,  94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "M" ); i++;
  initPopulate(i, jwin_ctext_proc,      209, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "M" ); i++;
  initPopulate(i, jwin_ctext_proc,      80,  94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "M" ); i++;
  initPopulate(i, jwin_ctext_proc,      209, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "M" ); i++;
  initPopulate(i, jwin_ctext_proc,      80,  94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "M" ); i++;
  initPopulate(i, jwin_ctext_proc,      209, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "M" ); i++;
  initPopulate(i, jwin_ctext_proc,      80,  94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "M" ); i++;
  initPopulate(i, jwin_ctext_proc,      209, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "M" ); i++;
  initPopulate(i, jwin_ctext_proc,      80,  94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "M" ); i++;
  initPopulate(i, jwin_ctext_proc,      209, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "M" ); i++;
  initPopulate(i, jwin_ctext_proc,      80,  94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "M" ); i++;
  initPopulate(i, jwin_ctext_proc,      209, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "M" ); i++;
  initPopulate(i, jwin_ctext_proc,      80,  94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "M" ); i++;
  initPopulate(i, jwin_ctext_proc,      209, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "M" ); i++;
  initPopulate(i, jwin_ctext_proc,      80,  94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "M" ); i++;
  initPopulate(i, jwin_ctext_proc,      209, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "M" ); i++;
  initPopulate(i, jwin_ctext_proc,      80,  94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "M" ); i++;
  initPopulate(i, jwin_ctext_proc,      209, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "M" ); i++;
  initPopulate(i, jwin_ctext_proc,      80,  94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "M" ); i++;
  initPopulate(i, jwin_ctext_proc,      209, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "M" ); i++;
  initPopulate(i, jwin_ctext_proc,      80,  94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "M" ); i++;
  initPopulate(i, jwin_ctext_proc,      209, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "M" ); i++;
  initPopulate(i, jwin_ctext_proc,      80,  94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "M" ); i++;
  initPopulate(i, jwin_ctext_proc,      209, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "M" ); i++;
  initPopulate(i, jwin_ctext_proc,      80,  94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "M" ); i++;
  initPopulate(i, jwin_ctext_proc,      209, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "M" ); i++;
  initPopulate(i, jwin_ctext_proc,      80,  94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "M" ); i++;
  initPopulate(i, jwin_ctext_proc,      209, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "M" ); i++;
  initPopulate(i, jwin_ctext_proc,      80,  94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "M" ); i++;
  initPopulate(i, jwin_ctext_proc,      209, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "M" ); i++;
  initPopulate(i, jwin_ctext_proc,      80,  94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "M" ); i++;
  initPopulate(i, jwin_ctext_proc,      209, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "M" ); i++;
  initPopulate(i, jwin_ctext_proc,      80,  94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "M" ); i++;
  initPopulate(i, jwin_ctext_proc,      209, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "M" ); i++;
  initPopulate(i, jwin_ctext_proc,      80,  94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "M" ); i++;
  initPopulate(i, jwin_ctext_proc,      209, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "M" ); i++;
  initPopulate(i, jwin_ctext_proc,      80,  94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "M" ); i++;
  initPopulate(i, jwin_ctext_proc,      209, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "M" ); i++;
  initPopulate(i, jwin_ctext_proc,      80,  94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "M" ); i++;
  initPopulate(i, jwin_ctext_proc,      209, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "M" ); i++;
  initPopulate(i, jwin_ctext_proc,      80,  94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "M" ); i++;
  initPopulate(i, jwin_ctext_proc,      209, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "M" ); i++;
  initPopulate(i, jwin_ctext_proc,      80,  94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "M" ); i++;
  initPopulate(i, jwin_ctext_proc,      209, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "M" ); i++;
  initPopulate(i, jwin_ctext_proc,      80,  94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "M" ); i++;
  initPopulate(i, jwin_ctext_proc,      209, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "M" ); i++;
  initPopulate(i, jwin_ctext_proc,      80,  94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "M" ); i++;
  initPopulate(i, jwin_ctext_proc,      209, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "M" ); i++;
  initPopulate(i, jwin_ctext_proc,      80,  94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "M" ); i++;
  initPopulate(i, jwin_ctext_proc,      209, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "M" ); i++;

  // 219
  initPopulate(i, jwin_ctext_proc,      92,  94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "C" ); i++;
  initPopulate(i, jwin_ctext_proc,      221, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "C" ); i++;
  initPopulate(i, jwin_ctext_proc,      92,  94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "C" ); i++;
  initPopulate(i, jwin_ctext_proc,      221, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "C" ); i++;
  initPopulate(i, jwin_ctext_proc,      92,  94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "C" ); i++;
  initPopulate(i, jwin_ctext_proc,      221, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "C" ); i++;
  initPopulate(i, jwin_ctext_proc,      92,  94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "C" ); i++;
  initPopulate(i, jwin_ctext_proc,      221, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "C" ); i++;
  initPopulate(i, jwin_ctext_proc,      92,  94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "C" ); i++;
  initPopulate(i, jwin_ctext_proc,      221, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "C" ); i++;
  initPopulate(i, jwin_ctext_proc,      92,  94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "C" ); i++;
  initPopulate(i, jwin_ctext_proc,      221, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "C" ); i++;
  initPopulate(i, jwin_ctext_proc,      92,  94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "C" ); i++;
  initPopulate(i, jwin_ctext_proc,      221, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "C" ); i++;
  initPopulate(i, jwin_ctext_proc,      92,  94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "C" ); i++;
  initPopulate(i, jwin_ctext_proc,      221, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "C" ); i++;
  initPopulate(i, jwin_ctext_proc,      92,  94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "C" ); i++;
  initPopulate(i, jwin_ctext_proc,      221, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "C" ); i++;
  initPopulate(i, jwin_ctext_proc,      92,  94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "C" ); i++;
  initPopulate(i, jwin_ctext_proc,      221, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "C" ); i++;
  initPopulate(i, jwin_ctext_proc,      92,  94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "C" ); i++;
  initPopulate(i, jwin_ctext_proc,      221, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "C" ); i++;
  initPopulate(i, jwin_ctext_proc,      92,  94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "C" ); i++;
  initPopulate(i, jwin_ctext_proc,      221, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "C" ); i++;
  initPopulate(i, jwin_ctext_proc,      92,  94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "C" ); i++;
  initPopulate(i, jwin_ctext_proc,      221, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "C" ); i++;
  initPopulate(i, jwin_ctext_proc,      92,  94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "C" ); i++;
  initPopulate(i, jwin_ctext_proc,      221, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "C" ); i++;
  initPopulate(i, jwin_ctext_proc,      92,  94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "C" ); i++;
  initPopulate(i, jwin_ctext_proc,      221, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "C" ); i++;
  initPopulate(i, jwin_ctext_proc,      92,  94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "C" ); i++;
  initPopulate(i, jwin_ctext_proc,      221, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "C" ); i++;
  initPopulate(i, jwin_ctext_proc,      92,  94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "C" ); i++;
  initPopulate(i, jwin_ctext_proc,      221, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "C" ); i++;
  initPopulate(i, jwin_ctext_proc,      92,  94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "C" ); i++;
  initPopulate(i, jwin_ctext_proc,      221, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "C" ); i++;
  initPopulate(i, jwin_ctext_proc,      92,  94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "C" ); i++;
  initPopulate(i, jwin_ctext_proc,      221, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "C" ); i++;
  initPopulate(i, jwin_ctext_proc,      92,  94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "C" ); i++;
  initPopulate(i, jwin_ctext_proc,      221, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "C" ); i++;
  initPopulate(i, jwin_ctext_proc,      92,  94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "C" ); i++;
  initPopulate(i, jwin_ctext_proc,      221, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "C" ); i++;
  initPopulate(i, jwin_ctext_proc,      92,  94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "C" ); i++;
  initPopulate(i, jwin_ctext_proc,      221, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "C" ); i++;
  initPopulate(i, jwin_ctext_proc,      92,  94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "C" ); i++;
  initPopulate(i, jwin_ctext_proc,      221, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "C" ); i++;
  initPopulate(i, jwin_ctext_proc,      92,  94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "C" ); i++;
  initPopulate(i, jwin_ctext_proc,      221, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "C" ); i++;
  initPopulate(i, jwin_ctext_proc,      92,  94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "C" ); i++;
  initPopulate(i, jwin_ctext_proc,      221, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "C" ); i++;
  initPopulate(i, jwin_ctext_proc,      92,  94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "C" ); i++;
  initPopulate(i, jwin_ctext_proc,      221, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "C" ); i++;



  // 271
  initPopulate(i, jwin_ctext_proc,      105, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "B" ); i++;
  initPopulate(i, jwin_ctext_proc,      234, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "B" ); i++;
  initPopulate(i, jwin_ctext_proc,      105, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "B" ); i++;
  initPopulate(i, jwin_ctext_proc,      234, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "B" ); i++;
  initPopulate(i, jwin_ctext_proc,      105, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "B" ); i++;
  initPopulate(i, jwin_ctext_proc,      234, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "B" ); i++;
  initPopulate(i, jwin_ctext_proc,      105, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "B" ); i++;
  initPopulate(i, jwin_ctext_proc,      234, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "B" ); i++;
  initPopulate(i, jwin_ctext_proc,      105, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "B" ); i++;
  initPopulate(i, jwin_ctext_proc,      234, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "B" ); i++;
  initPopulate(i, jwin_ctext_proc,      105, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "B" ); i++;
  initPopulate(i, jwin_ctext_proc,      234, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "B" ); i++;
  initPopulate(i, jwin_ctext_proc,      105, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "B" ); i++;
  initPopulate(i, jwin_ctext_proc,      234, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "B" ); i++;
  initPopulate(i, jwin_ctext_proc,      105, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "B" ); i++;
  initPopulate(i, jwin_ctext_proc,      234, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "B" ); i++;
  initPopulate(i, jwin_ctext_proc,      105, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "B" ); i++;
  initPopulate(i, jwin_ctext_proc,      234, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "B" ); i++;
  initPopulate(i, jwin_ctext_proc,      105, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "B" ); i++;
  initPopulate(i, jwin_ctext_proc,      234, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "B" ); i++;
  initPopulate(i, jwin_ctext_proc,      105, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "B" ); i++;
  initPopulate(i, jwin_ctext_proc,      234, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "B" ); i++;
  initPopulate(i, jwin_ctext_proc,      105, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "B" ); i++;
  initPopulate(i, jwin_ctext_proc,      234, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "B" ); i++;
  initPopulate(i, jwin_ctext_proc,      105, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "B" ); i++;
  initPopulate(i, jwin_ctext_proc,      234, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "B" ); i++;
  initPopulate(i, jwin_ctext_proc,      105, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "B" ); i++;
  initPopulate(i, jwin_ctext_proc,      234, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "B" ); i++;
  initPopulate(i, jwin_ctext_proc,      105, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "B" ); i++;
  initPopulate(i, jwin_ctext_proc,      234, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "B" ); i++;
  initPopulate(i, jwin_ctext_proc,      105, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "B" ); i++;
  initPopulate(i, jwin_ctext_proc,      234, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "B" ); i++;
  initPopulate(i, jwin_ctext_proc,      105, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "B" ); i++;
  initPopulate(i, jwin_ctext_proc,      234, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "B" ); i++;
  initPopulate(i, jwin_ctext_proc,      105, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "B" ); i++;
  initPopulate(i, jwin_ctext_proc,      234, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "B" ); i++;
  initPopulate(i, jwin_ctext_proc,      105, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "B" ); i++;
  initPopulate(i, jwin_ctext_proc,      234, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "B" ); i++;
  initPopulate(i, jwin_ctext_proc,      105, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "B" ); i++;
  initPopulate(i, jwin_ctext_proc,      234, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "B" ); i++;
  initPopulate(i, jwin_ctext_proc,      105, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "B" ); i++;
  initPopulate(i, jwin_ctext_proc,      234, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "B" ); i++;
  initPopulate(i, jwin_ctext_proc,      105, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "B" ); i++;
  initPopulate(i, jwin_ctext_proc,      234, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "B" ); i++;
  initPopulate(i, jwin_ctext_proc,      105, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "B" ); i++;
  initPopulate(i, jwin_ctext_proc,      234, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "B" ); i++;
  initPopulate(i, jwin_ctext_proc,      105, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "B" ); i++;
  initPopulate(i, jwin_ctext_proc,      234, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "B" ); i++;
  initPopulate(i, jwin_ctext_proc,      105, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "B" ); i++;
  initPopulate(i, jwin_ctext_proc,      234, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "B" ); i++;
  initPopulate(i, jwin_ctext_proc,      105, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "B" ); i++;
  initPopulate(i, jwin_ctext_proc,      234, 94,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "B" ); i++;



  // 323
  initPopulate(i, jwin_ctext_proc,      124, 94,  48,    9,   vc(0),  vc(11),    0,       D_DISABLED,           0,        0,     (void *) "K" ); i++;
  initPopulate(i, jwin_ctext_proc,      253, 94,  48,    9,   vc(0),  vc(11),    0,       D_DISABLED,           0,        0,     (void *) "K" ); i++;
  initPopulate(i, jwin_ctext_proc,      124, 94,  48,    9,   vc(0),  vc(11),    0,       D_DISABLED,           0,        0,     (void *) "K" ); i++;
  initPopulate(i, jwin_ctext_proc,      253, 94,  48,    9,   vc(0),  vc(11),    0,       D_DISABLED,           0,        0,     (void *) "K" ); i++;
  initPopulate(i, jwin_ctext_proc,      124, 94,  48,    9,   vc(0),  vc(11),    0,       D_DISABLED,           0,        0,     (void *) "K" ); i++;
  initPopulate(i, jwin_ctext_proc,      253, 94,  48,    9,   vc(0),  vc(11),    0,       D_DISABLED,           0,        0,     (void *) "K" ); i++;
  initPopulate(i, jwin_ctext_proc,      124, 94,  48,    9,   vc(0),  vc(11),    0,       D_DISABLED,           0,        0,     (void *) "K" ); i++;
  initPopulate(i, jwin_ctext_proc,      253, 94,  48,    9,   vc(0),  vc(11),    0,       D_DISABLED,           0,        0,     (void *) "K" ); i++;
  initPopulate(i, jwin_ctext_proc,      124, 94,  48,    9,   vc(0),  vc(11),    0,       D_DISABLED,           0,        0,     (void *) "K" ); i++;
  initPopulate(i, jwin_ctext_proc,      253, 94,  48,    9,   vc(0),  vc(11),    0,       D_DISABLED,           0,        0,     (void *) "K" ); i++;
  initPopulate(i, jwin_ctext_proc,      124, 94,  48,    9,   vc(0),  vc(11),    0,       D_DISABLED,           0,        0,     (void *) "K" ); i++;
  initPopulate(i, jwin_ctext_proc,      253, 94,  48,    9,   vc(0),  vc(11),    0,       D_DISABLED,           0,        0,     (void *) "K" ); i++;
  initPopulate(i, jwin_ctext_proc,      124, 94,  48,    9,   vc(0),  vc(11),    0,       D_DISABLED,           0,        0,     (void *) "K" ); i++;
  initPopulate(i, jwin_ctext_proc,      253, 94,  48,    9,   vc(0),  vc(11),    0,       D_DISABLED,           0,        0,     (void *) "K" ); i++;
  initPopulate(i, jwin_ctext_proc,      124, 94,  48,    9,   vc(0),  vc(11),    0,       D_DISABLED,           0,        0,     (void *) "K" ); i++;
  initPopulate(i, jwin_ctext_proc,      253, 94,  48,    9,   vc(0),  vc(11),    0,       D_DISABLED,           0,        0,     (void *) "K" ); i++;
  initPopulate(i, jwin_ctext_proc,      124, 94,  48,    9,   vc(0),  vc(11),    0,       D_DISABLED,           0,        0,     (void *) "K" ); i++;
  initPopulate(i, jwin_ctext_proc,      253, 94,  48,    9,   vc(0),  vc(11),    0,       D_DISABLED,           0,        0,     (void *) "K" ); i++;
  initPopulate(i, jwin_ctext_proc,      124, 94,  48,    9,   vc(0),  vc(11),    0,       D_DISABLED,           0,        0,     (void *) "K" ); i++;
  initPopulate(i, jwin_ctext_proc,      253, 94,  48,    9,   vc(0),  vc(11),    0,       D_DISABLED,           0,        0,     (void *) "K" ); i++;
  initPopulate(i, jwin_ctext_proc,      124, 94,  48,    9,   vc(0),  vc(11),    0,       D_DISABLED,           0,        0,     (void *) "K" ); i++;
  initPopulate(i, jwin_ctext_proc,      253, 94,  48,    9,   vc(0),  vc(11),    0,       D_DISABLED,           0,        0,     (void *) "K" ); i++;
  initPopulate(i, jwin_ctext_proc,      124, 94,  48,    9,   vc(0),  vc(11),    0,       D_DISABLED,           0,        0,     (void *) "K" ); i++;
  initPopulate(i, jwin_ctext_proc,      253, 94,  48,    9,   vc(0),  vc(11),    0,       D_DISABLED,           0,        0,     (void *) "K" ); i++;
  initPopulate(i, jwin_ctext_proc,      124, 94,  48,    9,   vc(0),  vc(11),    0,       D_DISABLED,           0,        0,     (void *) "K" ); i++;
  initPopulate(i, jwin_ctext_proc,      253, 94,  48,    9,   vc(0),  vc(11),    0,       D_DISABLED,           0,        0,     (void *) "K" ); i++;
  initPopulate(i, jwin_ctext_proc,      124, 94,  48,    9,   vc(0),  vc(11),    0,       D_DISABLED,           0,        0,     (void *) "K" ); i++;
  initPopulate(i, jwin_ctext_proc,      253, 94,  48,    9,   vc(0),  vc(11),    0,       D_DISABLED,           0,        0,     (void *) "K" ); i++;
  initPopulate(i, jwin_ctext_proc,      124, 94,  48,    9,   vc(0),  vc(11),    0,       D_DISABLED,           0,        0,     (void *) "K" ); i++;
  initPopulate(i, jwin_ctext_proc,      253, 94,  48,    9,   vc(0),  vc(11),    0,       D_DISABLED,           0,        0,     (void *) "K" ); i++;
  initPopulate(i, jwin_ctext_proc,      124, 94,  48,    9,   vc(0),  vc(11),    0,       D_DISABLED,           0,        0,     (void *) "K" ); i++;
  initPopulate(i, jwin_ctext_proc,      253, 94,  48,    9,   vc(0),  vc(11),    0,       D_DISABLED,           0,        0,     (void *) "K" ); i++;
  initPopulate(i, jwin_ctext_proc,      124, 94,  48,    9,   vc(0),  vc(11),    0,       D_DISABLED,           0,        0,     (void *) "K" ); i++;
  initPopulate(i, jwin_ctext_proc,      253, 94,  48,    9,   vc(0),  vc(11),    0,       D_DISABLED,           0,        0,     (void *) "K" ); i++;
  initPopulate(i, jwin_ctext_proc,      124, 94,  48,    9,   vc(0),  vc(11),    0,       D_DISABLED,           0,        0,     (void *) "K" ); i++;
  initPopulate(i, jwin_ctext_proc,      253, 94,  48,    9,   vc(0),  vc(11),    0,       D_DISABLED,           0,        0,     (void *) "K" ); i++;
  initPopulate(i, jwin_ctext_proc,      124, 94,  48,    9,   vc(0),  vc(11),    0,       D_DISABLED,           0,        0,     (void *) "K" ); i++;
  initPopulate(i, jwin_ctext_proc,      253, 94,  48,    9,   vc(0),  vc(11),    0,       D_DISABLED,           0,        0,     (void *) "K" ); i++;
  initPopulate(i, jwin_ctext_proc,      124, 94,  48,    9,   vc(0),  vc(11),    0,       D_DISABLED,           0,        0,     (void *) "K" ); i++;
  initPopulate(i, jwin_ctext_proc,      253, 94,  48,    9,   vc(0),  vc(11),    0,       D_DISABLED,           0,        0,     (void *) "K" ); i++;
  initPopulate(i, jwin_ctext_proc,      124, 94,  48,    9,   vc(0),  vc(11),    0,       D_DISABLED,           0,        0,     (void *) "K" ); i++;
  initPopulate(i, jwin_ctext_proc,      253, 94,  48,    9,   vc(0),  vc(11),    0,       D_DISABLED,           0,        0,     (void *) "K" ); i++;
  initPopulate(i, jwin_ctext_proc,      124, 94,  48,    9,   vc(0),  vc(11),    0,       D_DISABLED,           0,        0,     (void *) "K" ); i++;
  initPopulate(i, jwin_ctext_proc,      253, 94,  48,    9,   vc(0),  vc(11),    0,       D_DISABLED,           0,        0,     (void *) "K" ); i++;
  initPopulate(i, jwin_ctext_proc,      124, 94,  48,    9,   vc(0),  vc(11),    0,       D_DISABLED,           0,        0,     (void *) "K" ); i++;
  initPopulate(i, jwin_ctext_proc,      253, 94,  48,    9,   vc(0),  vc(11),    0,       D_DISABLED,           0,        0,     (void *) "K" ); i++;
  initPopulate(i, jwin_ctext_proc,      124, 94,  48,    9,   vc(0),  vc(11),    0,       D_DISABLED,           0,        0,     (void *) "K" ); i++;
  initPopulate(i, jwin_ctext_proc,      253, 94,  48,    9,   vc(0),  vc(11),    0,       D_DISABLED,           0,        0,     (void *) "K" ); i++;
  initPopulate(i, jwin_ctext_proc,      124, 94,  48,    9,   vc(0),  vc(11),    0,       D_DISABLED,           0,        0,     (void *) "K" ); i++;
  initPopulate(i, jwin_ctext_proc,      253, 94,  48,    9,   vc(0),  vc(11),    0,       D_DISABLED,           0,        0,     (void *) "K" ); i++;
  initPopulate(i, jwin_ctext_proc,      124, 94,  48,    9,   vc(0),  vc(11),    0,       D_DISABLED,           0,        0,     (void *) "K" ); i++;
  initPopulate(i, jwin_ctext_proc,      253, 94,  48,    9,   vc(0),  vc(11),    0,       D_DISABLED,           0,        0,     (void *) "K" ); i++;



  // 375
  initPopulate(i, jwin_rtext_proc,      52,  106,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "0" ); i++;
  initPopulate(i, jwin_rtext_proc,      52,  124,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "1" ); i++;
  initPopulate(i, jwin_rtext_proc,      52,  142,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "2" ); i++;
  initPopulate(i, jwin_rtext_proc,      52,  160,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "3" ); i++;
  initPopulate(i, jwin_rtext_proc,      52,  178,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "4" ); i++;
  initPopulate(i, jwin_rtext_proc,      181, 106,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "5" ); i++;
  initPopulate(i, jwin_rtext_proc,      181, 124,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "6" ); i++;
  initPopulate(i, jwin_rtext_proc,      181, 142,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "7" ); i++;
  initPopulate(i, jwin_rtext_proc,      181, 160,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "8" ); i++;
  initPopulate(i, jwin_rtext_proc,      181, 178,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "9" ); i++;
  // 385
  initPopulate(i, jwin_rtext_proc,      52,  106,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "10" ); i++;
  initPopulate(i, jwin_rtext_proc,      52,  124,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "11" ); i++;
  initPopulate(i, jwin_rtext_proc,      52,  142,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "12" ); i++;
  initPopulate(i, jwin_rtext_proc,      52,  160,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "13" ); i++;
  initPopulate(i, jwin_rtext_proc,      52,  178,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "14" ); i++;
  initPopulate(i, jwin_rtext_proc,      181, 106,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "15" ); i++;
  initPopulate(i, jwin_rtext_proc,      181, 124,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "16" ); i++;
  initPopulate(i, jwin_rtext_proc,      181, 142,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "17" ); i++;
  initPopulate(i, jwin_rtext_proc,      181, 160,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "18" ); i++;
  initPopulate(i, jwin_rtext_proc,      181, 178,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "19" ); i++;
  // 395
  initPopulate(i, jwin_rtext_proc,      52,  106,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "20" ); i++;
  initPopulate(i, jwin_rtext_proc,      52,  124,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "21" ); i++;
  initPopulate(i, jwin_rtext_proc,      52,  142,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "22" ); i++;
  initPopulate(i, jwin_rtext_proc,      52,  160,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "23" ); i++;
  initPopulate(i, jwin_rtext_proc,      52,  178,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "24" ); i++;
  initPopulate(i, jwin_rtext_proc,      181, 106,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "25" ); i++;
  initPopulate(i, jwin_rtext_proc,      181, 124,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "26" ); i++;
  initPopulate(i, jwin_rtext_proc,      181, 142,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "27" ); i++;
  initPopulate(i, jwin_rtext_proc,      181, 160,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "28" ); i++;
  initPopulate(i, jwin_rtext_proc,      181, 178,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "29" ); i++;
  // 405
  initPopulate(i, jwin_rtext_proc,      52,  106,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "30" ); i++;
  initPopulate(i, jwin_rtext_proc,      52,  124,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "31" ); i++;
  initPopulate(i, jwin_rtext_proc,      52,  142,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "32" ); i++;
  initPopulate(i, jwin_rtext_proc,      52,  160,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "33" ); i++;
  initPopulate(i, jwin_rtext_proc,      52,  178,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "34" ); i++;
  initPopulate(i, jwin_rtext_proc,      181, 106,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "35" ); i++;
  initPopulate(i, jwin_rtext_proc,      181, 124,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "36" ); i++;
  initPopulate(i, jwin_rtext_proc,      181, 142,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "37" ); i++;
  initPopulate(i, jwin_rtext_proc,      181, 160,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "38" ); i++;
  initPopulate(i, jwin_rtext_proc,      181, 178,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "39" ); i++;
  // 415
  initPopulate(i, jwin_rtext_proc,      52,  106,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "40" ); i++;
  initPopulate(i, jwin_rtext_proc,      52,  124,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "41" ); i++;
  initPopulate(i, jwin_rtext_proc,      52,  142,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "42" ); i++;
  initPopulate(i, jwin_rtext_proc,      52,  160,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "43" ); i++;
  initPopulate(i, jwin_rtext_proc,      52,  178,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "44" ); i++;
  initPopulate(i, jwin_rtext_proc,      181, 106,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "45" ); i++;
  initPopulate(i, jwin_rtext_proc,      181, 124,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "46" ); i++;
  initPopulate(i, jwin_rtext_proc,      181, 142,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "47" ); i++;
  initPopulate(i, jwin_rtext_proc,      181, 160,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "48" ); i++;
  initPopulate(i, jwin_rtext_proc,      181, 178,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "49" ); i++;
  // 425
  initPopulate(i, jwin_rtext_proc,      52,  106,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "50" ); i++;
  initPopulate(i, jwin_rtext_proc,      52,  124,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "51" ); i++;
  initPopulate(i, jwin_rtext_proc,      52,  142,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "52" ); i++;
  initPopulate(i, jwin_rtext_proc,      52,  160,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "53" ); i++;
  initPopulate(i, jwin_rtext_proc,      52,  178,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "54" ); i++;
  initPopulate(i, jwin_rtext_proc,      181, 106,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "55" ); i++;
  initPopulate(i, jwin_rtext_proc,      181, 124,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "56" ); i++;
  initPopulate(i, jwin_rtext_proc,      181, 142,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "57" ); i++;
  initPopulate(i, jwin_rtext_proc,      181, 160,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "58" ); i++;
  initPopulate(i, jwin_rtext_proc,      181, 178,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "59" ); i++;
  // 435
  initPopulate(i, jwin_rtext_proc,      52,  106,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "60" ); i++;
  initPopulate(i, jwin_rtext_proc,      52,  124,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "61" ); i++;
  initPopulate(i, jwin_rtext_proc,      52,  142,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "62" ); i++;
  initPopulate(i, jwin_rtext_proc,      52,  160,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "63" ); i++;
  initPopulate(i, jwin_rtext_proc,      52,  178,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "64" ); i++;
  initPopulate(i, jwin_rtext_proc,      181, 106,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "65" ); i++;
  initPopulate(i, jwin_rtext_proc,      181, 124,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "66" ); i++;
  initPopulate(i, jwin_rtext_proc,      181, 142,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "67" ); i++;
  initPopulate(i, jwin_rtext_proc,      181, 160,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "68" ); i++;
  initPopulate(i, jwin_rtext_proc,      181, 178,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "69" ); i++;
  // 445
  initPopulate(i, jwin_rtext_proc,      52,  106,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "70" ); i++;
  initPopulate(i, jwin_rtext_proc,      52,  124,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "71" ); i++;
  initPopulate(i, jwin_rtext_proc,      52,  142,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "72" ); i++;
  initPopulate(i, jwin_rtext_proc,      52,  160,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "73" ); i++;
  initPopulate(i, jwin_rtext_proc,      52,  178,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "74" ); i++;
  initPopulate(i, jwin_rtext_proc,      181, 106,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "75" ); i++;
  initPopulate(i, jwin_rtext_proc,      181, 124,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "76" ); i++;
  initPopulate(i, jwin_rtext_proc,      181, 142,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "77" ); i++;
  initPopulate(i, jwin_rtext_proc,      181, 160,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "78" ); i++;
  initPopulate(i, jwin_rtext_proc,      181, 178,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "79" ); i++;
  // 455
  initPopulate(i, jwin_rtext_proc,      52,  106,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "80" ); i++;
  initPopulate(i, jwin_rtext_proc,      52,  124,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "81" ); i++;
  initPopulate(i, jwin_rtext_proc,      52,  142,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "82" ); i++;
  initPopulate(i, jwin_rtext_proc,      52,  160,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "83" ); i++;
  initPopulate(i, jwin_rtext_proc,      52,  178,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "84" ); i++;
  initPopulate(i, jwin_rtext_proc,      181, 106,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "85" ); i++;
  initPopulate(i, jwin_rtext_proc,      181, 124,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "86" ); i++;
  initPopulate(i, jwin_rtext_proc,      181, 142,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "87" ); i++;
  initPopulate(i, jwin_rtext_proc,      181, 160,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "88" ); i++;
  initPopulate(i, jwin_rtext_proc,      181, 178,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "89" ); i++;
  // 465
  initPopulate(i, jwin_rtext_proc,      52,  106,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "90" ); i++;
  initPopulate(i, jwin_rtext_proc,      52,  124,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "91" ); i++;
  initPopulate(i, jwin_rtext_proc,      52,  142,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "92" ); i++;
  initPopulate(i, jwin_rtext_proc,      52,  160,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "93" ); i++;
  initPopulate(i, jwin_rtext_proc,      52,  178,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "94" ); i++;
  initPopulate(i, jwin_rtext_proc,      181, 106,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "95" ); i++;
  initPopulate(i, jwin_rtext_proc,      181, 124,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "96" ); i++;
  initPopulate(i, jwin_rtext_proc,      181, 142,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "97" ); i++;
  initPopulate(i, jwin_rtext_proc,      181, 160,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "98" ); i++;
  initPopulate(i, jwin_rtext_proc,      181, 178,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "99" ); i++;
  // 475
  initPopulate(i, jwin_rtext_proc,      52,  106,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "100" ); i++;
  initPopulate(i, jwin_rtext_proc,      52,  124,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "101" ); i++;
  initPopulate(i, jwin_rtext_proc,      52,  142,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "102" ); i++;
  initPopulate(i, jwin_rtext_proc,      52,  160,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "103" ); i++;
  initPopulate(i, jwin_rtext_proc,      52,  178,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "104" ); i++;
  initPopulate(i, jwin_rtext_proc,      181, 106,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "105" ); i++;
  initPopulate(i, jwin_rtext_proc,      181, 124,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "106" ); i++;
  initPopulate(i, jwin_rtext_proc,      181, 142,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "107" ); i++;
  initPopulate(i, jwin_rtext_proc,      181, 160,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "108" ); i++;
  initPopulate(i, jwin_rtext_proc,      181, 178,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "109" ); i++;
  // 485
  initPopulate(i, jwin_rtext_proc,      52,  106,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "110" ); i++;
  initPopulate(i, jwin_rtext_proc,      52,  124,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "111" ); i++;
  initPopulate(i, jwin_rtext_proc,      52,  142,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "112" ); i++;
  initPopulate(i, jwin_rtext_proc,      52,  160,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "113" ); i++;
  initPopulate(i, jwin_rtext_proc,      52,  178,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "114" ); i++;
  initPopulate(i, jwin_rtext_proc,      181, 106,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "115" ); i++;
  initPopulate(i, jwin_rtext_proc,      181, 124,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "116" ); i++;
  initPopulate(i, jwin_rtext_proc,      181, 142,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "117" ); i++;
  initPopulate(i, jwin_rtext_proc,      181, 160,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "118" ); i++;
  initPopulate(i, jwin_rtext_proc,      181, 178,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "119" ); i++;
  // 495
  initPopulate(i, jwin_rtext_proc,      52,  106,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "120" ); i++;
  initPopulate(i, jwin_rtext_proc,      52,  124,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "121" ); i++;
  initPopulate(i, jwin_rtext_proc,      52,  142,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "122" ); i++;
  initPopulate(i, jwin_rtext_proc,      52,  160,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "123" ); i++;
  initPopulate(i, jwin_rtext_proc,      52,  178,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "124" ); i++;
  initPopulate(i, jwin_rtext_proc,      181, 106,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "125" ); i++;
  initPopulate(i, jwin_rtext_proc,      181, 124,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "126" ); i++;
  initPopulate(i, jwin_rtext_proc,      181, 142,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "127" ); i++;
  initPopulate(i, jwin_rtext_proc,      181, 160,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "128" ); i++;
  initPopulate(i, jwin_rtext_proc,      181, 178,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "129" ); i++;
  // 505
  initPopulate(i, jwin_rtext_proc,      52,  106,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "130" ); i++;
  initPopulate(i, jwin_rtext_proc,      52,  124,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "131" ); i++;
  initPopulate(i, jwin_rtext_proc,      52,  142,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "132" ); i++;
  initPopulate(i, jwin_rtext_proc,      52,  160,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "133" ); i++;
  initPopulate(i, jwin_rtext_proc,      52,  178,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "134" ); i++;
  initPopulate(i, jwin_rtext_proc,      181, 106,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "135" ); i++;
  initPopulate(i, jwin_rtext_proc,      181, 124,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "136" ); i++;
  initPopulate(i, jwin_rtext_proc,      181, 142,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "137" ); i++;
  initPopulate(i, jwin_rtext_proc,      181, 160,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "138" ); i++;
  initPopulate(i, jwin_rtext_proc,      181, 178,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "139" ); i++;
  // 515
  initPopulate(i, jwin_rtext_proc,      52,  106,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "140" ); i++;
  initPopulate(i, jwin_rtext_proc,      52,  124,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "141" ); i++;
  initPopulate(i, jwin_rtext_proc,      52,  142,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "142" ); i++;
  initPopulate(i, jwin_rtext_proc,      52,  160,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "143" ); i++;
  initPopulate(i, jwin_rtext_proc,      52,  178,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "144" ); i++;
  initPopulate(i, jwin_rtext_proc,      181, 106,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "145" ); i++;
  initPopulate(i, jwin_rtext_proc,      181, 124,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "146" ); i++;
  initPopulate(i, jwin_rtext_proc,      181, 142,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "147" ); i++;
  initPopulate(i, jwin_rtext_proc,      181, 160,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "148" ); i++;
  initPopulate(i, jwin_rtext_proc,      181, 178,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "149" ); i++;
  // 525
  initPopulate(i, jwin_rtext_proc,      52,  106,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "150" ); i++;
  initPopulate(i, jwin_rtext_proc,      52,  124,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "151" ); i++;
  initPopulate(i, jwin_rtext_proc,      52,  142,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "152" ); i++;
  initPopulate(i, jwin_rtext_proc,      52,  160,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "153" ); i++;
  initPopulate(i, jwin_rtext_proc,      52,  178,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "154" ); i++;
  initPopulate(i, jwin_rtext_proc,      181, 106,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "155" ); i++;
  initPopulate(i, jwin_rtext_proc,      181, 124,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "156" ); i++;
  initPopulate(i, jwin_rtext_proc,      181, 142,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "157" ); i++;
  initPopulate(i, jwin_rtext_proc,      181, 160,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "158" ); i++;
  initPopulate(i, jwin_rtext_proc,      181, 178,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "159" ); i++;
  // 535
  initPopulate(i, jwin_rtext_proc,      52,  106,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "160" ); i++;
  initPopulate(i, jwin_rtext_proc,      52,  124,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "161" ); i++;
  initPopulate(i, jwin_rtext_proc,      52,  142,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "162" ); i++;
  initPopulate(i, jwin_rtext_proc,      52,  160,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "163" ); i++;
  initPopulate(i, jwin_rtext_proc,      52,  178,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "164" ); i++;
  initPopulate(i, jwin_rtext_proc,      181, 106,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "165" ); i++;
  initPopulate(i, jwin_rtext_proc,      181, 124,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "166" ); i++;
  initPopulate(i, jwin_rtext_proc,      181, 142,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "167" ); i++;
  initPopulate(i, jwin_rtext_proc,      181, 160,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "168" ); i++;
  initPopulate(i, jwin_rtext_proc,      181, 178,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "169" ); i++;
  // 545
  initPopulate(i, jwin_rtext_proc,      52,  106,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "170" ); i++;
  initPopulate(i, jwin_rtext_proc,      52,  124,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "171" ); i++;
  initPopulate(i, jwin_rtext_proc,      52,  142,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "172" ); i++;
  initPopulate(i, jwin_rtext_proc,      52,  160,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "173" ); i++;
  initPopulate(i, jwin_rtext_proc,      52,  178,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "174" ); i++;
  initPopulate(i, jwin_rtext_proc,      181, 106,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "175" ); i++;
  initPopulate(i, jwin_rtext_proc,      181, 124,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "176" ); i++;
  initPopulate(i, jwin_rtext_proc,      181, 142,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "177" ); i++;
  initPopulate(i, jwin_rtext_proc,      181, 160,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "178" ); i++;
  initPopulate(i, jwin_rtext_proc,      181, 178,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "179" ); i++;
  // 555
  initPopulate(i, jwin_rtext_proc,      52,  106,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "180" ); i++;
  initPopulate(i, jwin_rtext_proc,      52,  124,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "181" ); i++;
  initPopulate(i, jwin_rtext_proc,      52,  142,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "182" ); i++;
  initPopulate(i, jwin_rtext_proc,      52,  160,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "183" ); i++;
  initPopulate(i, jwin_rtext_proc,      52,  178,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "184" ); i++;
  initPopulate(i, jwin_rtext_proc,      181, 106,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "185" ); i++;
  initPopulate(i, jwin_rtext_proc,      181, 124,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "186" ); i++;
  initPopulate(i, jwin_rtext_proc,      181, 142,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "187" ); i++;
  initPopulate(i, jwin_rtext_proc,      181, 160,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "188" ); i++;
  initPopulate(i, jwin_rtext_proc,      181, 178,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "189" ); i++;
  // 565
  initPopulate(i, jwin_rtext_proc,      52,  106,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "190" ); i++;
  initPopulate(i, jwin_rtext_proc,      52,  124,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "191" ); i++;
  initPopulate(i, jwin_rtext_proc,      52,  142,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "192" ); i++;
  initPopulate(i, jwin_rtext_proc,      52,  160,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "193" ); i++;
  initPopulate(i, jwin_rtext_proc,      52,  178,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "194" ); i++;
  initPopulate(i, jwin_rtext_proc,      181, 106,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "195" ); i++;
  initPopulate(i, jwin_rtext_proc,      181, 124,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "196" ); i++;
  initPopulate(i, jwin_rtext_proc,      181, 142,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "197" ); i++;
  initPopulate(i, jwin_rtext_proc,      181, 160,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "198" ); i++;
  initPopulate(i, jwin_rtext_proc,      181, 178,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "199" ); i++;
  // 575
  initPopulate(i, jwin_rtext_proc,      52,  106,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "200" ); i++;
  initPopulate(i, jwin_rtext_proc,      52,  124,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "201" ); i++;
  initPopulate(i, jwin_rtext_proc,      52,  142,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "202" ); i++;
  initPopulate(i, jwin_rtext_proc,      52,  160,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "203" ); i++;
  initPopulate(i, jwin_rtext_proc,      52,  178,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "204" ); i++;
  initPopulate(i, jwin_rtext_proc,      181, 106,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "205" ); i++;
  initPopulate(i, jwin_rtext_proc,      181, 124,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "206" ); i++;
  initPopulate(i, jwin_rtext_proc,      181, 142,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "207" ); i++;
  initPopulate(i, jwin_rtext_proc,      181, 160,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "208" ); i++;
  initPopulate(i, jwin_rtext_proc,      181, 178,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "209" ); i++;
  // 585
  initPopulate(i, jwin_rtext_proc,      52,  106,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "210" ); i++;
  initPopulate(i, jwin_rtext_proc,      52,  124,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "211" ); i++;
  initPopulate(i, jwin_rtext_proc,      52,  142,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "212" ); i++;
  initPopulate(i, jwin_rtext_proc,      52,  160,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "213" ); i++;
  initPopulate(i, jwin_rtext_proc,      52,  178,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "214" ); i++;
  initPopulate(i, jwin_rtext_proc,      181, 106,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "215" ); i++;
  initPopulate(i, jwin_rtext_proc,      181, 124,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "216" ); i++;
  initPopulate(i, jwin_rtext_proc,      181, 142,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "217" ); i++;
  initPopulate(i, jwin_rtext_proc,      181, 160,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "218" ); i++;
  initPopulate(i, jwin_rtext_proc,      181, 178,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "219" ); i++;
  // 595
  initPopulate(i, jwin_rtext_proc,      52,  106,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "220" ); i++;
  initPopulate(i, jwin_rtext_proc,      52,  124,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "221" ); i++;
  initPopulate(i, jwin_rtext_proc,      52,  142,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "222" ); i++;
  initPopulate(i, jwin_rtext_proc,      52,  160,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "223" ); i++;
  initPopulate(i, jwin_rtext_proc,      52,  178,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "224" ); i++;
  initPopulate(i, jwin_rtext_proc,      181, 106,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "225" ); i++;
  initPopulate(i, jwin_rtext_proc,      181, 124,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "226" ); i++;
  initPopulate(i, jwin_rtext_proc,      181, 142,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "227" ); i++;
  initPopulate(i, jwin_rtext_proc,      181, 160,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "228" ); i++;
  initPopulate(i, jwin_rtext_proc,      181, 178,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "229" ); i++;
  // 605
  initPopulate(i, jwin_rtext_proc,      52,  106,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "230" ); i++;
  initPopulate(i, jwin_rtext_proc,      52,  124,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "231" ); i++;
  initPopulate(i, jwin_rtext_proc,      52,  142,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "232" ); i++;
  initPopulate(i, jwin_rtext_proc,      52,  160,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "233" ); i++;
  initPopulate(i, jwin_rtext_proc,      52,  178,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "234" ); i++;
  initPopulate(i, jwin_rtext_proc,      181, 106,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "235" ); i++;
  initPopulate(i, jwin_rtext_proc,      181, 124,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "236" ); i++;
  initPopulate(i, jwin_rtext_proc,      181, 142,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "237" ); i++;
  initPopulate(i, jwin_rtext_proc,      181, 160,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "238" ); i++;
  initPopulate(i, jwin_rtext_proc,      181, 178,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "239" ); i++;
  // 615
  initPopulate(i, jwin_rtext_proc,      52,  106,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "240" ); i++;
  initPopulate(i, jwin_rtext_proc,      52,  124,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "241" ); i++;
  initPopulate(i, jwin_rtext_proc,      52,  142,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "242" ); i++;
  initPopulate(i, jwin_rtext_proc,      52,  160,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "243" ); i++;
  initPopulate(i, jwin_rtext_proc,      52,  178,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "244" ); i++;
  initPopulate(i, jwin_rtext_proc,      181, 106,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "245" ); i++;
  initPopulate(i, jwin_rtext_proc,      181, 124,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "246" ); i++;
  initPopulate(i, jwin_rtext_proc,      181, 142,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "247" ); i++;
  initPopulate(i, jwin_rtext_proc,      181, 160,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "248" ); i++;
  initPopulate(i, jwin_rtext_proc,      181, 178,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "249" ); i++;
  // 625
  initPopulate(i, jwin_rtext_proc,      52,  106,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "250" ); i++;
  initPopulate(i, jwin_rtext_proc,      52,  124,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "251" ); i++;
  initPopulate(i, jwin_rtext_proc,      52,  142,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "252" ); i++;
  initPopulate(i, jwin_rtext_proc,      52,  160,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "253" ); i++;
  initPopulate(i, jwin_rtext_proc,      52,  178,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "254" ); i++;
  initPopulate(i, jwin_rtext_proc,      181, 106,  48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "255" ); i++;



  // 631
  initPopulate(i, jwin_check_proc,      75,  106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      75,  124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      75,  142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      75,  160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      75,  178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      204, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      204, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      204, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      204, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      204, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  // 641
  initPopulate(i, jwin_check_proc,      75,  106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      75,  124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      75,  142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      75,  160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      75,  178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      204, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      204, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      204, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      204, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      204, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  // 651
  initPopulate(i, jwin_check_proc,      75,  106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      75,  124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      75,  142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      75,  160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      75,  178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      204, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      204, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      204, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      204, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      204, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  // 661
  initPopulate(i, jwin_check_proc,      75,  106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      75,  124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      75,  142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      75,  160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      75,  178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      204, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      204, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      204, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      204, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      204, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  // 671
  initPopulate(i, jwin_check_proc,      75,  106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      75,  124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      75,  142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      75,  160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      75,  178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      204, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      204, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      204, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      204, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      204, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  // 681
  initPopulate(i, jwin_check_proc,      75,  106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      75,  124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      75,  142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      75,  160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      75,  178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      204, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      204, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      204, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      204, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      204, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  // 691
  initPopulate(i, jwin_check_proc,      75,  106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      75,  124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      75,  142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      75,  160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      75,  178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      204, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      204, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      204, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      204, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      204, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  // 701
  initPopulate(i, jwin_check_proc,      75,  106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      75,  124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      75,  142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      75,  160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      75,  178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      204, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      204, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      204, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      204, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      204, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  // 711
  initPopulate(i, jwin_check_proc,      75,  106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      75,  124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      75,  142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      75,  160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      75,  178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      204, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      204, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      204, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      204, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      204, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  // 721
  initPopulate(i, jwin_check_proc,      75,  106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      75,  124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      75,  142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      75,  160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      75,  178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      204, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      204, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      204, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      204, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      204, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  // 731
  initPopulate(i, jwin_check_proc,      75,  106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      75,  124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      75,  142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      75,  160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      75,  178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      204, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      204, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      204, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      204, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      204, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  // 741
  initPopulate(i, jwin_check_proc,      75,  106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      75,  124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      75,  142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      75,  160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      75,  178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      204, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      204, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      204, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      204, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      204, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  // 751
  initPopulate(i, jwin_check_proc,      75,  106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      75,  124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      75,  142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      75,  160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      75,  178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      204, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      204, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      204, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      204, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      204, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  // 761
  initPopulate(i, jwin_check_proc,      75,  106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      75,  124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      75,  142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      75,  160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      75,  178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      204, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      204, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      204, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      204, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      204, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  // 771
  initPopulate(i, jwin_check_proc,      75,  106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      75,  124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      75,  142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      75,  160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      75,  178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      204, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      204, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      204, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      204, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      204, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  // 781
  initPopulate(i, jwin_check_proc,      75,  106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      75,  124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      75,  142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      75,  160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      75,  178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      204, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      204, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      204, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      204, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      204, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  // 791
  initPopulate(i, jwin_check_proc,      75,  106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      75,  124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      75,  142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      75,  160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      75,  178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      204, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      204, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      204, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      204, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      204, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  // 801
  initPopulate(i, jwin_check_proc,      75,  106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      75,  124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      75,  142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      75,  160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      75,  178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      204, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      204, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      204, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      204, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      204, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  // 811
  initPopulate(i, jwin_check_proc,      75,  106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      75,  124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      75,  142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      75,  160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      75,  178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      204, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      204, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      204, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      204, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      204, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  // 821
  initPopulate(i, jwin_check_proc,      75,  106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      75,  124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      75,  142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      75,  160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      75,  178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      204, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      204, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      204, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      204, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      204, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  // 831
  initPopulate(i, jwin_check_proc,      75,  106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      75,  124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      75,  142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      75,  160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      75,  178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      204, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      204, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      204, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      204, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      204, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  // 841
  initPopulate(i, jwin_check_proc,      75,  106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      75,  124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      75,  142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      75,  160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      75,  178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      204, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      204, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      204, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      204, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      204, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  // 851
  initPopulate(i, jwin_check_proc,      75,  106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      75,  124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      75,  142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      75,  160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      75,  178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      204, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      204, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      204, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      204, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      204, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  // 861
  initPopulate(i, jwin_check_proc,      75,  106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      75,  124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      75,  142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      75,  160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      75,  178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      204, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      204, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      204, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      204, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      204, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  // 871
  initPopulate(i, jwin_check_proc,      75,  106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      75,  124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      75,  142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      75,  160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      75,  178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      204, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      204, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      204, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      204, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      204, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  // 881
  initPopulate(i, jwin_check_proc,      75,  106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      75,  124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      75,  142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      75,  160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      75,  178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      204, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;



  // 887
  initPopulate(i, jwin_check_proc,      88,  106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      88,  124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      88,  142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      88,  160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      88,  178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      217, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      217, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      217, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      217, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      217, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  // 897
  initPopulate(i, jwin_check_proc,      88,  106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      88,  124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      88,  142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      88,  160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      88,  178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      217, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      217, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      217, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      217, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      217, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  // 907
  initPopulate(i, jwin_check_proc,      88,  106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      88,  124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      88,  142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      88,  160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      88,  178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      217, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      217, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      217, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      217, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      217, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  // 917
  initPopulate(i, jwin_check_proc,      88,  106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      88,  124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      88,  142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      88,  160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      88,  178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      217, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      217, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      217, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      217, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      217, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  // 927
  initPopulate(i, jwin_check_proc,      88,  106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      88,  124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      88,  142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      88,  160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      88,  178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      217, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      217, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      217, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      217, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      217, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  // 937
  initPopulate(i, jwin_check_proc,      88,  106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      88,  124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      88,  142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      88,  160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      88,  178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      217, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      217, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      217, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      217, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      217, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  // 947
  initPopulate(i, jwin_check_proc,      88,  106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      88,  124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      88,  142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      88,  160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      88,  178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      217, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      217, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      217, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      217, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      217, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  // 957
  initPopulate(i, jwin_check_proc,      88,  106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      88,  124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      88,  142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      88,  160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      88,  178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      217, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      217, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      217, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      217, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      217, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  // 967
  initPopulate(i, jwin_check_proc,      88,  106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      88,  124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      88,  142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      88,  160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      88,  178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      217, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      217, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      217, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      217, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      217, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  // 977
  initPopulate(i, jwin_check_proc,      88,  106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      88,  124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      88,  142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      88,  160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      88,  178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      217, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      217, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      217, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      217, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      217, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  // 987
  initPopulate(i, jwin_check_proc,      88,  106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      88,  124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      88,  142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      88,  160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      88,  178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      217, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      217, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      217, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      217, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      217, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  // 997
  initPopulate(i, jwin_check_proc,      88,  106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      88,  124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      88,  142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      88,  160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      88,  178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      217, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      217, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      217, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      217, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      217, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  // 1007
  initPopulate(i, jwin_check_proc,      88,  106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      88,  124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      88,  142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      88,  160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      88,  178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      217, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      217, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      217, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      217, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      217, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  // 1017
  initPopulate(i, jwin_check_proc,      88,  106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      88,  124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      88,  142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      88,  160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      88,  178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      217, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      217, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      217, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      217, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      217, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  // 1027
  initPopulate(i, jwin_check_proc,      88,  106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      88,  124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      88,  142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      88,  160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      88,  178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      217, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      217, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      217, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      217, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      217, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  // 1037
  initPopulate(i, jwin_check_proc,      88,  106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      88,  124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      88,  142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      88,  160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      88,  178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      217, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      217, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      217, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      217, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      217, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  // 1047
  initPopulate(i, jwin_check_proc,      88,  106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      88,  124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      88,  142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      88,  160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      88,  178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      217, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      217, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      217, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      217, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      217, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  // 1057
  initPopulate(i, jwin_check_proc,      88,  106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      88,  124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      88,  142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      88,  160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      88,  178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      217, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      217, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      217, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      217, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      217, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  // 1067
  initPopulate(i, jwin_check_proc,      88,  106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      88,  124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      88,  142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      88,  160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      88,  178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      217, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      217, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      217, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      217, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      217, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  // 1077
  initPopulate(i, jwin_check_proc,      88,  106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      88,  124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      88,  142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      88,  160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      88,  178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      217, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      217, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      217, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      217, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      217, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  // 1087
  initPopulate(i, jwin_check_proc,      88,  106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      88,  124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      88,  142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      88,  160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      88,  178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      217, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      217, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      217, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      217, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      217, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  // 1097
  initPopulate(i, jwin_check_proc,      88,  106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      88,  124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      88,  142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      88,  160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      88,  178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      217, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      217, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      217, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      217, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      217, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  // 1107
  initPopulate(i, jwin_check_proc,      88,  106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      88,  124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      88,  142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      88,  160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      88,  178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      217, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      217, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      217, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      217, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      217, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  // 1117
  initPopulate(i, jwin_check_proc,      88,  106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      88,  124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      88,  142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      88,  160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      88,  178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      217, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      217, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      217, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      217, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      217, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  // 1127
  initPopulate(i, jwin_check_proc,      88,  106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      88,  124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      88,  142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      88,  160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      88,  178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      217, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      217, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      217, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      217, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      217, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  // 1137
  initPopulate(i, jwin_check_proc,      88,  106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      88,  124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      88,  142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      88,  160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      88,  178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      217, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;




  // 1143
  initPopulate(i, jwin_check_proc,      101, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      101, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      101, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      101, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      101, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      230, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      230, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      230, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      230, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      230, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  // 1153
  initPopulate(i, jwin_check_proc,      101, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      101, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      101, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      101, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      101, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      230, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      230, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      230, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      230, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      230, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  // 1163
  initPopulate(i, jwin_check_proc,      101, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      101, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      101, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      101, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      101, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      230, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      230, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      230, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      230, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      230, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  // 1173
  initPopulate(i, jwin_check_proc,      101, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      101, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      101, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      101, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      101, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      230, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      230, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      230, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      230, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      230, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  // 1183
  initPopulate(i, jwin_check_proc,      101, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      101, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      101, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      101, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      101, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      230, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      230, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      230, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      230, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      230, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  // 1193
  initPopulate(i, jwin_check_proc,      101, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      101, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      101, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      101, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      101, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      230, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      230, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      230, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      230, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      230, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  // 1203
  initPopulate(i, jwin_check_proc,      101, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      101, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      101, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      101, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      101, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      230, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      230, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      230, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      230, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      230, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  // 1213
  initPopulate(i, jwin_check_proc,      101, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      101, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      101, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      101, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      101, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      230, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      230, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      230, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      230, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      230, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  // 1223
  initPopulate(i, jwin_check_proc,      101, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      101, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      101, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      101, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      101, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      230, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      230, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      230, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      230, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      230, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  // 1233
  initPopulate(i, jwin_check_proc,      101, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      101, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      101, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      101, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      101, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      230, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      230, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      230, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      230, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      230, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  // 1243
  initPopulate(i, jwin_check_proc,      101, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      101, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      101, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      101, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      101, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      230, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      230, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      230, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      230, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      230, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  // 1253
  initPopulate(i, jwin_check_proc,      101, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      101, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      101, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      101, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      101, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      230, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      230, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      230, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      230, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      230, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  // 1263
  initPopulate(i, jwin_check_proc,      101, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      101, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      101, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      101, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      101, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      230, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      230, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      230, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      230, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      230, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  // 1273
  initPopulate(i, jwin_check_proc,      101, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      101, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      101, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      101, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      101, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      230, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      230, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      230, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      230, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      230, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  // 1283
  initPopulate(i, jwin_check_proc,      101, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      101, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      101, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      101, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      101, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      230, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      230, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      230, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      230, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      230, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  // 1293
  initPopulate(i, jwin_check_proc,      101, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      101, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      101, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      101, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      101, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      230, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      230, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      230, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      230, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      230, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  // 1303
  initPopulate(i, jwin_check_proc,      101, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      101, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      101, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      101, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      101, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      230, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      230, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      230, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      230, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      230, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  // 1313
  initPopulate(i, jwin_check_proc,      101, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      101, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      101, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      101, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      101, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      230, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      230, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      230, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      230, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      230, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  // 1323
  initPopulate(i, jwin_check_proc,      101, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      101, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      101, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      101, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      101, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      230, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      230, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      230, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      230, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      230, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  // 1333
  initPopulate(i, jwin_check_proc,      101, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      101, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      101, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      101, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      101, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      230, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      230, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      230, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      230, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      230, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  // 1343
  initPopulate(i, jwin_check_proc,      101, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      101, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      101, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      101, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      101, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      230, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      230, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      230, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      230, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      230, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  // 1353
  initPopulate(i, jwin_check_proc,      101, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      101, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      101, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      101, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      101, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      230, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      230, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      230, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      230, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      230, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  // 1363
  initPopulate(i, jwin_check_proc,      101, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      101, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      101, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      101, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      101, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      230, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      230, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      230, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      230, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      230, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  // 1373
  initPopulate(i, jwin_check_proc,      101, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      101, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      101, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      101, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      101, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      230, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      230, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      230, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      230, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      230, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  // 1383
  initPopulate(i, jwin_check_proc,      101, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      101, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      101, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      101, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      101, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      230, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      230, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      230, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      230, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      230, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  // 1393
  initPopulate(i, jwin_check_proc,      101, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      101, 124,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      101, 142,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      101, 160,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      101, 178,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,      230, 106,  64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     NULL ); i++;





  //1399
  initPopulate(i, jwin_edit_proc,       114, 102, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       114, 120, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       114, 138, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       114, 156, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       114, 174, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       243, 102, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       243, 120, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       243, 138, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       243, 156, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       243, 174, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  //1409
  initPopulate(i, jwin_edit_proc,       114, 102, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       114, 120, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       114, 138, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       114, 156, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       114, 174, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       243, 102, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       243, 120, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       243, 138, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       243, 156, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       243, 174, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  //1419
  initPopulate(i, jwin_edit_proc,       114, 102, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       114, 120, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       114, 138, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       114, 156, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       114, 174, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       243, 102, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       243, 120, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       243, 138, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       243, 156, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       243, 174, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  //1429
  initPopulate(i, jwin_edit_proc,       114, 102, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       114, 120, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       114, 138, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       114, 156, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       114, 174, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       243, 102, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       243, 120, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       243, 138, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       243, 156, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       243, 174, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  //1439
  initPopulate(i, jwin_edit_proc,       114, 102, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       114, 120, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       114, 138, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       114, 156, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       114, 174, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       243, 102, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       243, 120, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       243, 138, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       243, 156, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       243, 174, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  //1449
  initPopulate(i, jwin_edit_proc,       114, 102, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       114, 120, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       114, 138, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       114, 156, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       114, 174, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       243, 102, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       243, 120, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       243, 138, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       243, 156, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       243, 174, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  //1459
  initPopulate(i, jwin_edit_proc,       114, 102, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       114, 120, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       114, 138, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       114, 156, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       114, 174, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       243, 102, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       243, 120, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       243, 138, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       243, 156, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       243, 174, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  //1469
  initPopulate(i, jwin_edit_proc,       114, 102, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       114, 120, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       114, 138, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       114, 156, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       114, 174, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       243, 102, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       243, 120, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       243, 138, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       243, 156, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       243, 174, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  //1479
  initPopulate(i, jwin_edit_proc,       114, 102, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       114, 120, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       114, 138, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       114, 156, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       114, 174, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       243, 102, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       243, 120, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       243, 138, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       243, 156, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       243, 174, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  //1489
  initPopulate(i, jwin_edit_proc,       114, 102, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       114, 120, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       114, 138, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       114, 156, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       114, 174, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       243, 102, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       243, 120, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       243, 138, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       243, 156, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       243, 174, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  //1499
  initPopulate(i, jwin_edit_proc,       114, 102, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       114, 120, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       114, 138, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       114, 156, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       114, 174, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       243, 102, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       243, 120, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       243, 138, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       243, 156, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       243, 174, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  //1509
  initPopulate(i, jwin_edit_proc,       114, 102, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       114, 120, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       114, 138, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       114, 156, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       114, 174, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       243, 102, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       243, 120, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       243, 138, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       243, 156, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       243, 174, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  //1519
  initPopulate(i, jwin_edit_proc,       114, 102, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       114, 120, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       114, 138, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       114, 156, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       114, 174, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       243, 102, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       243, 120, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       243, 138, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       243, 156, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       243, 174, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  //1529
  initPopulate(i, jwin_edit_proc,       114, 102, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       114, 120, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       114, 138, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       114, 156, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       114, 174, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       243, 102, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       243, 120, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       243, 138, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       243, 156, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       243, 174, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  //1539
  initPopulate(i, jwin_edit_proc,       114, 102, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       114, 120, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       114, 138, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       114, 156, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       114, 174, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       243, 102, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       243, 120, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       243, 138, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       243, 156, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       243, 174, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  //1549
  initPopulate(i, jwin_edit_proc,       114, 102, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       114, 120, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       114, 138, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       114, 156, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       114, 174, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       243, 102, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       243, 120, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       243, 138, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       243, 156, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       243, 174, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  //1559
  initPopulate(i, jwin_edit_proc,       114, 102, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       114, 120, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       114, 138, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       114, 156, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       114, 174, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       243, 102, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       243, 120, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       243, 138, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       243, 156, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       243, 174, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  //1569
  initPopulate(i, jwin_edit_proc,       114, 102, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       114, 120, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       114, 138, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       114, 156, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       114, 174, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       243, 102, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       243, 120, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       243, 138, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       243, 156, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       243, 174, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  //1579
  initPopulate(i, jwin_edit_proc,       114, 102, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       114, 120, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       114, 138, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       114, 156, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       114, 174, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       243, 102, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       243, 120, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       243, 138, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       243, 156, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       243, 174, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  //1589
  initPopulate(i, jwin_edit_proc,       114, 102, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       114, 120, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       114, 138, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       114, 156, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       114, 174, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       243, 102, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       243, 120, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       243, 138, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       243, 156, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       243, 174, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  //1599
  initPopulate(i, jwin_edit_proc,       114, 102, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       114, 120, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       114, 138, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       114, 156, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       114, 174, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       243, 102, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       243, 120, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       243, 138, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       243, 156, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       243, 174, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  //1609
  initPopulate(i, jwin_edit_proc,       114, 102, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       114, 120, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       114, 138, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       114, 156, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       114, 174, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       243, 102, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       243, 120, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       243, 138, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       243, 156, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       243, 174, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  //1619
  initPopulate(i, jwin_edit_proc,       114, 102, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       114, 120, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       114, 138, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       114, 156, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       114, 174, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       243, 102, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       243, 120, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       243, 138, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       243, 156, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       243, 174, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  //1629
  initPopulate(i, jwin_edit_proc,       114, 102, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       114, 120, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       114, 138, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       114, 156, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       114, 174, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       243, 102, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       243, 120, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       243, 138, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       243, 156, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       243, 174, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  //1639
  initPopulate(i, jwin_edit_proc,       114, 102, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       114, 120, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       114, 138, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       114, 156, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       114, 174, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       243, 102, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       243, 120, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       243, 138, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       243, 156, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       243, 174, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  //1649
  initPopulate(i, jwin_edit_proc,       114, 102, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       114, 120, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       114, 138, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       114, 156, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       114, 174, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,       243, 102, 29,   16,       0,       0,    0,       0,  3,        0,     NULL ); i++;
  //1655
  initPopulate(i, jwin_text_proc,        25,  54,   128,   8,    vc(15),  vc(1),  0,       0,          0,             0,        (void *) "Starting DMap: " ); i++;
  initPopulate(i, d_dropdmaplist_proc,   94,  50,   180,  16,   jwin_pal[jcTEXTFG],  jwin_pal[jcTEXTBG],  0,       0,          0,             0,       (void *) dmaplist, NULL, startdmapxy ); i++;

  //1657
  initPopulate(i, jwin_text_proc,       25,   72,  144,   8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "Heart Containers:" ); i++;
  initPopulate(i, jwin_text_proc,       25,   90,  176,   8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "Starting HP (hearts):" ); i++;
  initPopulate(i, jwin_text_proc,       25,  108,  104,   8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "Continue HP:" ); i++;
  initPopulate(i, jwin_text_proc,       25,  126,   48,   8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "Keys:" ); i++;
  initPopulate(i, jwin_text_proc,       93,  126,   64,   8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "Rupies:" ); i++;

  //1662
  initPopulate(i, jwin_edit_proc,      108,   68,   21,   16,    vc(12),  vc(1),  0,       0,          2,             0,       NULL ); i++;
  initPopulate(i, jwin_edit_proc,      125,   86,   21,   16,    vc(12),  vc(1),  0,       0,          2,             0,       NULL ); i++;
  initPopulate(i, jwin_edit_proc,       84,  104,   21,   16,    vc(12),  vc(1),  0,       0,          3,             0,       NULL ); i++;
  initPopulate(i, jwin_edit_proc,       53,  122,   21,   16,    vc(12),  vc(1),  0,       0,          2,             0,       NULL ); i++;
  initPopulate(i, jwin_edit_proc,      128,  122,   21,   16,    vc(12),  vc(1),  0,       0,          3,             0,       NULL ); i++;
  initPopulate(i, jwin_check_proc,     107,  108,   25,    9,    vc(14),  vc(1),  0,       0,          1,             0,       (void *) "%" ); i++;
  //1668
  initPopulate(i, jwin_frame_proc,     164,   71,   40,   50,    0,       0,      0,       0,          FR_ETCHED,     0,       NULL ); i++;
  initPopulate(i, jwin_text_proc,      168,   68,   40,    8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) " HCP's " ); i++;
  initPopulate(i, jwin_radio_proc,     168,   78,   25,    9,    vc(14),  vc(1),  0,       0,          1,             0,       (void *) "0" ); i++;
  initPopulate(i, jwin_radio_proc,     168,   88,   25,    9,    vc(14),  vc(1),  0,       0,          1,             0,       (void *) "1" ); i++;
  initPopulate(i, jwin_radio_proc,     168,   98,   25,    9,    vc(14),  vc(1),  0,       0,          1,             0,       (void *) "2" ); i++;
  initPopulate(i, jwin_radio_proc,     168,  108,   25,    9,    vc(14),  vc(1),  0,       0,          1,             0,       (void *) "3" ); i++;
  //1674
  initPopulate(i, jwin_frame_proc,     221,   71,   62,  50,    0,       0,      0,       0,           FR_ETCHED,     0,       NULL ); i++;
  initPopulate(i, jwin_text_proc,      225,   68,    0,   8,    vc(15),  vc(1),  0,       0,           0,             0,       (void *) " Triforce " ); i++;
  initPopulate(i, jwin_check_proc,     229,   78,   25,   9,    vc(14),  vc(1),  0,       0,           1,             0,       (void *) "1" ); i++;
  initPopulate(i, jwin_check_proc,     229,   88,   25,   9,    vc(14),  vc(1),  0,       0,           1,             0,       (void *) "2" ); i++;
  initPopulate(i, jwin_check_proc,     229,   98,   25,   9,    vc(14),  vc(1),  0,       0,           1,             0,       (void *) "3" ); i++;
  initPopulate(i, jwin_check_proc,     229,  108,   25,   9,    vc(14),  vc(1),  0,       0,           1,             0,       (void *) "4" ); i++;
  initPopulate(i, jwin_check_proc,     259,   78,   25,   9,    vc(14),  vc(1),  0,       0,           1,             0,       (void *) "5" ); i++;
  initPopulate(i, jwin_check_proc,     259,   88,   25,   9,    vc(14),  vc(1),  0,       0,           1,             0,       (void *) "6" ); i++;
  initPopulate(i, jwin_check_proc,     259,   98,   25,   9,    vc(14),  vc(1),  0,       0,           1,             0,       (void *) "7" ); i++;
  initPopulate(i, jwin_check_proc,     259,  108,   25,   9,    vc(14),  vc(1),  0,       0,           1,             0,       (void *) "8" ); i++;
  //1684
  initPopulate(i, jwin_check_proc,      25,  142,   17,   9,    vc(14),  vc(1),  0,       0,           1,             0,       (void *) "Slash" ); i++;
  initPopulate(i, jwin_frame_proc,     186,  125,   78,  50,    0,       0,      0,       0,           FR_ETCHED,     0,       NULL ); i++;
  initPopulate(i, jwin_text_proc,      190,  122,  113,   9,    vc(0),   vc(11), 0,       0,           0,             0,       (void *) " Magic " ); i++;
  initPopulate(i, jwin_ctext_proc,     205,  132,   73,   9,    vc(0),   vc(11), 0,       0,           0,             0,       (void *) "Start" ); i++;
  initPopulate(i, jwin_ctext_proc,     246,  132,   73,   9,    vc(0),   vc(11), 0,       0,           0,             0,       (void *) "Max" ); i++;
  initPopulate(i, jwin_edit_proc,      194,  142,   21,  16,    0,       0,      0,       0,           2,             0,       NULL ); i++;
  initPopulate(i, jwin_edit_proc,      235,  142,   21,  16,    0,       0,      0,       0,           2,             0,       NULL ); i++;
  initPopulate(i, jwin_check_proc,     194,  162,   25,   9,    vc(14),  vc(1),  0,       0,           1,             0,       (void *) "Double" ); i++;
  //1692

  initPopulate(i, jwin_frame_proc,       163,   181,  131,   20,       0,       0,    0,       0,   FR_ETCHED,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,       169,   186,   64,    9,   vc(0),  vc(11),    0,       0,          1,             0,     (void *) "Cane of Byrna" ); i++;
  initPopulate(i, jwin_text_proc,       80,  144,   64,   8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "Max Rupies:" ); i++;
  initPopulate(i, jwin_edit_proc,      128,  140,   21,   16,    vc(12),  vc(1),  0,       0,          5,             0,       NULL ); i++;
  initPopulate(i, jwin_text_proc,       80,  162,   64,   8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "Max Keys:" ); i++;
  initPopulate(i, jwin_edit_proc,      128,  158,   21,   16,    vc(12),  vc(1),  0,       0,          5,             0,       NULL ); i++;
  initPopulate(i, d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL ); i++;
  initPopulate(i, d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL ); i++;
  initPopulate(i, d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL ); i++;
  initPopulate(i, d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL ); i++;
  initPopulate(i, d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL ); i++;
  initPopulate(i, d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL ); i++;
  initPopulate(i, d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL ); i++;
  initPopulate(i, d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL ); i++;
  initPopulate(i, d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL ); i++;
  initPopulate(i, d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL ); i++;
  initPopulate(i, d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL ); i++;
  initPopulate(i, d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL ); i++;
  initPopulate(i, d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL ); i++;
  initPopulate(i, d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL ); i++;
  initPopulate(i, d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL ); i++;
  initPopulate(i, d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL ); i++;
  initPopulate(i, d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL ); i++;
  initPopulate(i, d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL ); i++;
  initPopulate(i, d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL ); i++;
  initPopulate(i, d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL ); i++;
  initPopulate(i, d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL ); i++;
  initPopulate(i, d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL ); i++;
  initPopulate(i, d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL ); i++;
  initPopulate(i, d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL ); i++;
  initPopulate(i, d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL ); i++;
  initPopulate(i, d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL ); i++;
  initPopulate(i, d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL ); i++;
  initPopulate(i, d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL ); i++;
  initPopulate(i, d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL ); i++;
  initPopulate(i, d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL ); i++;
  initPopulate(i, d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL ); i++;
  initPopulate(i, d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL ); i++;
  initPopulate(i, d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL ); i++;
  initPopulate(i, d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL ); i++;
  initPopulate(i, d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL ); i++;
  initPopulate(i, d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL ); i++;
  initPopulate(i, d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL ); i++;
  initPopulate(i, d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL ); i++;
  initPopulate(i, d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL ); i++;
  initPopulate(i, d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL ); i++;
  initPopulate(i, d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL ); i++;
  initPopulate(i, d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL ); i++;
  initPopulate(i, d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL ); i++;
  initPopulate(i, d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL ); i++;
  initPopulate(i, d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL ); i++;
  initPopulate(i, d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL ); i++;
  initPopulate(i, d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL ); i++;
  initPopulate(i, d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL ); i++;
  initPopulate(i, d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL ); i++;
  initPopulate(i, d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL ); i++;
  initPopulate(i, d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL ); i++;
  initPopulate(i, d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL ); i++;
  initPopulate(i, d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL ); i++;
  initPopulate(i, d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL ); i++;
  /**/
  //1752
  init_dlg[1752].proc = NULL;
}
