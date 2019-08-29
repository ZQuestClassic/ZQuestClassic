//--------------------------------------------------------
//  Zelda Classic
//  by Jeremy Craner, 1999-2000
//
//  zquest.cc
//
//  Main code for the quest editor.
//
//--------------------------------------------------------

#ifndef __GTHREAD_HIDE_WIN32API                             
#define __GTHREAD_HIDE_WIN32API 1
#endif                            //prevent indirectly including windows.h

#include <map>
#include <vector>
#include <algorithm>

#include <stdio.h>

#include "zc_alleg.h"
#include "jwin.h"
#include "zdefs.h"
#include "zsys.h"
#include "gui.h"

#define vc(x)  ((x)+224)                                    // offset to 'VGA color' x (row 14)

using namespace std;

extern int jwin_pal[jcMAX];
extern FONT *sfont2;
extern FONT *lfont;
extern FONT *pfont;
extern itemdata *itemsbuf;
extern byte quest_rules[20];
extern char *item_string[];

void initPopulate(int i, DIALOG_PROC proc, int x, int y, int w, int h, int fg, int bg, int key, int flags, int d1, int d2,
                  void *dp, void *dp2 = NULL, void *dp3 = NULL);

static const int endEquipField = 33;

void doFamily(int family, zinitdata *data);
int jwin_initlist_proc(int msg,DIALOG *d,int c);

class Family
{
public:
	Family(int fam, int lvl, int id) : family(fam), level(lvl), itemid(id) {}
	int family;
	int level;
	int itemid;
	bool operator<(const Family &other) const
	{
		return level < other.level;
	}
};

extern int d_dummy_proc(int msg,DIALOG *d,int c);
extern int d_dropdmaplist_proc(int msg,DIALOG *d,int c);
extern char *dmaplist(int index, int *list_size);
extern int onHelp();
extern int startdmapxy[6];
extern void onInitOK();

static map<int, vector<Family> > families;

static map<int, char *> famnames;

int d_line_proc(int msg, DIALOG *d, int c)
{
  //these are here to bypass compiler warnings about unused arguments
  c=c;

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
  { "000",             D_SELECTED,   init_dmap_items_000s_list,    0, NULL },
  { "100",             0,            init_dmap_items_100s_list,    0, NULL },
  { "200",             0,            init_dmap_items_200s_list,    0, NULL },
  { NULL,              0,            0,                            0, NULL }
};


static TABPANEL init_dmap_items_000s_tabs[] =
{
  // (text)
  { "00",              D_SELECTED,   init_dmap_items_0_00s_list,   0, NULL },
  { "10",              0,            init_dmap_items_0_10s_list,   0, NULL },
  { "20",              0,            init_dmap_items_0_20s_list,   0, NULL },
  { "30",              0,            init_dmap_items_0_30s_list,   0, NULL },
  { "40",              0,            init_dmap_items_0_40s_list,   0, NULL },
  { "50",              0,            init_dmap_items_0_50s_list,   0, NULL },
  { "60",              0,            init_dmap_items_0_60s_list,   0, NULL },
  { "70",              0,            init_dmap_items_0_70s_list,   0, NULL },
  { "80",              0,            init_dmap_items_0_80s_list,   0, NULL },
  { "90",              0,            init_dmap_items_0_90s_list,   0, NULL },
  { NULL,              0,            0,                            0, NULL }
};

static TABPANEL init_dmap_items_100s_tabs[] =
{
  // (text)
  { "00",              D_SELECTED,   init_dmap_items_1_00s_list,   0, NULL },
  { "10",              0,            init_dmap_items_1_10s_list,   0, NULL },
  { "20",              0,            init_dmap_items_1_20s_list,   0, NULL },
  { "30",              0,            init_dmap_items_1_30s_list,   0, NULL },
  { "40",              0,            init_dmap_items_1_40s_list,   0, NULL },
  { "50",              0,            init_dmap_items_1_50s_list,   0, NULL },
  { "60",              0,            init_dmap_items_1_60s_list,   0, NULL },
  { "70",              0,            init_dmap_items_1_70s_list,   0, NULL },
  { "80",              0,            init_dmap_items_1_80s_list,   0, NULL },
  { "90",              0,            init_dmap_items_1_90s_list,   0, NULL },
  { NULL,              0,            0,                            0, NULL }
};

static TABPANEL init_dmap_items_200s_tabs[] =
{
  // (text)
  { "00",              D_SELECTED,   init_dmap_items_2_00s_list,   0, NULL },
  { "10",              0,            init_dmap_items_2_10s_list,   0, NULL },
  { "20",              0,            init_dmap_items_2_20s_list,   0, NULL },
  { "30",              0,            init_dmap_items_2_30s_list,   0, NULL },
  { "40",              0,            init_dmap_items_2_40s_list,   0, NULL },
  { "50",              0,            init_dmap_items_2_50s_list,   0, NULL },
  { NULL,              0,            0,                            0, NULL }
};

TABPANEL init_tabs[] =
{
  // (text)
  { "Equipment",       D_SELECTED,   init_equipment_list,          0, NULL },
  { "Items",           0,            init_items_list,              0, NULL },
  { "Level Items",     0,            init_dmap_items_list,         0, NULL },
  { "Misc",            0,            init_misc_list,               0, NULL },
  { NULL,              0,            0,                            0, NULL }
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



DIALOG init_dlg[1753];

void initPopulate(int i, DIALOG_PROC Proc, int X, int Y, int W, int H, int FG, int BG, int Key, int Flags, int D1, int D2, void *DP, void *DP2, void *DP3)
{
  init_dlg[i].proc = Proc;
  init_dlg[i].x = X;
  init_dlg[i].y = Y;
  init_dlg[i].w = W;
  init_dlg[i].h = H;
  init_dlg[i].fg = FG;
  init_dlg[i].bg = BG;
  init_dlg[i].key = Key;
  init_dlg[i].flags = Flags;
  init_dlg[i].d1 = D1;
  init_dlg[i].d2 = D2;
  init_dlg[i].dp = DP;
  init_dlg[i].dp2 = DP2;
  init_dlg[i].dp3 = DP3;
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
  initPopulate(i, d_dummy_proc,  31,    50,   108,   150,       jwin_pal[jcTEXTFG],  jwin_pal[jcTEXTBG],    0,    0,   0,        0,     NULL ); i++;
  initPopulate(i, jwin_frame_proc,    140,    50,   155,  150,   vc(0),  vc(11),    0,       0,           FR_ETCHED,        0,     NULL ); i++;
  initPopulate(i, jwin_check_proc,    145,    60,   74,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "1"); i++;
  initPopulate(i, jwin_check_proc,    145,    70,   74,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "2" ); i++;
  initPopulate(i, jwin_check_proc,    145,    80,   74,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "3" ); i++;
  initPopulate(i, jwin_check_proc,    145,    90,   74,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "4" ); i++;
  // 11
  initPopulate(i, jwin_check_proc,    145,   100,   74,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "5" ); i++;
  initPopulate(i, jwin_check_proc,    145,   110,   74,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "6" ); i++;
  initPopulate(i, jwin_check_proc,    145,   120,   74,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "7" ); i++;
  initPopulate(i, jwin_check_proc,    145,   130,   74,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "8" ); i++;
  initPopulate(i, jwin_check_proc,    145,   140,   74,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "9" ); i++;
  // 16
  initPopulate(i, jwin_check_proc,    145,   150,   74,    9,   vc(0),  vc(11),    0,       0,			 1,        0,     (void *) "10" ); i++;
  initPopulate(i, jwin_check_proc,    145,   160,   74,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "11" ); i++;
  initPopulate(i, jwin_check_proc,    145,   170,   74,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "12" ); i++;
  initPopulate(i, jwin_check_proc,    145,   180,   74,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "13" ); i++;
  initPopulate(i, jwin_check_proc,    220,   60,    74,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "14" ); i++;
  // 21
  initPopulate(i, jwin_check_proc,    220,    70,   74,    9,   vc(0),  vc(11),    0,       0,			 1,        0,     (void *) "15" ); i++;
  initPopulate(i, jwin_check_proc,    220,    80,   74,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "16" ); i++;
  initPopulate(i, jwin_check_proc,    220,    90,   74,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "17" ); i++;
  initPopulate(i, jwin_check_proc,    220,    100,  74,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "18" ); i++;
  // 25
  initPopulate(i, jwin_check_proc,    220,    110,  74,    9,   vc(0),  vc(11),    0,       0,			 1,        0,     (void *) "19" ); i++;
  initPopulate(i, jwin_check_proc,    220,    120,  74,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "20" ); i++;
  initPopulate(i, jwin_check_proc,    220,    130,  74,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "21" ); i++;
  initPopulate(i, jwin_check_proc,    220,    140,  74,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "22" ); i++;
  // 29
  initPopulate(i, jwin_check_proc,    220,    150,  74,    9,   vc(0),  vc(11),    0,       0,			  1,        0,     (void *) "23" ); i++;
  initPopulate(i, jwin_check_proc,    220,    160,  74,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "24" ); i++;
  initPopulate(i, jwin_check_proc,    220,    170,  74,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "25" ); i++;
  initPopulate(i, jwin_check_proc,    220,    180,  74,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "26" ); i++;
  // 33
  initPopulate(i, d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL ); i++;
  initPopulate(i, d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL ); i++;
  initPopulate(i, d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL ); i++;
  initPopulate(i, d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL ); i++;
  // 37
  initPopulate(i, d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL ); i++;
  initPopulate(i, d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL ); i++;
  initPopulate(i, d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL ); i++;
  initPopulate(i, d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL ); i++;
  initPopulate(i, d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL ); i++;
  initPopulate(i, d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL ); i++;
  initPopulate(i, d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL ); i++;
  initPopulate(i, d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL ); i++;
  // 45
  initPopulate(i, d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL ); i++;
  initPopulate(i, d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL ); i++;
  initPopulate(i, d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL ); i++;
  initPopulate(i, d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL ); i++;
  initPopulate(i, d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL ); i++;
  // 49
  initPopulate(i, d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL ); i++;
  initPopulate(i, d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL ); i++;
  initPopulate(i, d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL ); i++;
  initPopulate(i, d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL ); i++;
  initPopulate(i, d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL ); i++;
  // 53
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
  // 65 (Items)
  initPopulate(i, d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL ); i++;
  initPopulate(i, d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL ); i++;
  initPopulate(i, d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL ); i++;
  initPopulate(i, d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL ); i++;
  initPopulate(i, d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL ); i++;
  // 70
  initPopulate(i, jwin_frame_proc,      163,   53,  86,     40,       0,       0,    0,       0,   FR_ETCHED,        0,     NULL ); i++;
  initPopulate(i, jwin_text_proc,       167,   50, 121,      9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) " Arrow " ); i++;
  initPopulate(i, d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL ); i++;
  initPopulate(i, d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL ); i++;
  initPopulate(i, d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL ); i++;
  // 75
  initPopulate(i, jwin_frame_proc,    27,    53,   48,   40,       0,       0,    0,       0,   FR_ETCHED,        0,     NULL ); i++;
  initPopulate(i, jwin_text_proc,     31,    50,   48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) " Potion " ); i++;
  initPopulate(i, jwin_radio_proc,    35,    60,   48,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "None" ); i++;
  initPopulate(i, jwin_radio_proc,    35,    70,   48,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Blue" ); i++;
  initPopulate(i, jwin_radio_proc,    35,    80,   48,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Red" ); i++;
  // 80
  initPopulate(i, d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL ); i++;
  initPopulate(i, d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL ); i++;
  initPopulate(i, d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL ); i++;
  // 83
  initPopulate(i, d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL ); i++;
  initPopulate(i, d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL ); i++;
  initPopulate(i, d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL ); i++;
  // 86
  initPopulate(i, d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL ); i++;
  initPopulate(i, d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL ); i++;
  initPopulate(i, d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL ); i++;
  initPopulate(i, d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL ); i++;
  // 90
  initPopulate(i, d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL ); i++;
  initPopulate(i, d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL ); i++;
  initPopulate(i, d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL ); i++;
  initPopulate(i, d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL ); i++;
  // 94
  initPopulate(i, d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL ); i++;
  initPopulate(i, d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL ); i++;
  initPopulate(i, d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL ); i++;
  initPopulate(i, d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL ); i++;
  initPopulate(i, d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL ); i++;
  initPopulate(i, d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL ); i++;
  initPopulate(i, d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL ); i++;
  initPopulate(i, d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL ); i++;
  initPopulate(i, d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL ); i++;
  // 103
  initPopulate(i, jwin_frame_proc,    76,   53,   86,   91,       0,       0,    0,       0,   FR_ETCHED,        0,     NULL ); i++;
  initPopulate(i, jwin_text_proc,     80,   50,  113,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) " Bombs " ); i++;
  // 105
  initPopulate(i, jwin_frame_proc,    80,   63,   78,   37,       0,       0,    0,       0,   FR_ETCHED,        0,     NULL ); i++;
  initPopulate(i, jwin_text_proc,     84,   60,   73,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) " Normal " ); i++;
  initPopulate(i, jwin_ctext_proc,    99,   70,   73,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "Start" ); i++;
  initPopulate(i, jwin_ctext_proc,   140,   70,   73,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "Max" ); i++;
  // 109
  initPopulate(i, jwin_edit_proc,     88,  80,   21,   16,       0,       0,    0,       0,           2,        0,     NULL ); i++;
  initPopulate(i, d_maxbombsedit_proc, 129, 80,  21,   16,       0,       0,    0,       0,           2,        0,     NULL ); i++;
  initPopulate(i, jwin_frame_proc,    80,   103,  78,   37,       0,       0,    0,       0,   FR_ETCHED,        0,     NULL ); i++;
  initPopulate(i, jwin_text_proc,     84,   101,  73,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) " Super " ); i++;
  initPopulate(i, jwin_ctext_proc,    99,   111,  73,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "Start" ); i++;
  initPopulate(i, jwin_ctext_proc,   140,   111,  73,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "Max" ); i++;
  // 115
  initPopulate(i, jwin_edit_proc,     88,  121,   21,   16,       0,       0,    0,       0,           2,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,    129,  121,   21,   16,       0,       0,    0,       D_DISABLED,  2,        0,     NULL ); i++;
  // 117
  initPopulate(i, jwin_ctext_proc,   182,    60,   73,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "Start" ); i++;
  initPopulate(i, jwin_ctext_proc,   223,    60,   73,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "Max" ); i++;
  initPopulate(i, jwin_edit_proc,    171,    70,   21,   16,       0,       0,    0,       0,           2,        0,     NULL ); i++;
  initPopulate(i, jwin_edit_proc,    212,    70,   21,   16,       0,       0,    0,       0,           2,        0,     NULL ); i++;
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

  initPopulate(i, d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL ); i++;
  initPopulate(i, d_dummy_proc,           0,    0,   0,      0,      0,       0,      0,       0,          0,             0,       NULL ); i++;
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

const char *itype_names[] = { "Swords", "Boomerangs", "Arrows", "Candles", "Whistles",
"Baits", "Letters", "Potions", "Wands", "Rings", "Wallets", "Amulets", "Shields", "Bows", "Rafts",
"Ladders", "Books", "Magic Keys", "Bracelets", "Flippers", "Boots", "Hookshots", "Lenses", "Hammers",
"Din's Fires", "Farore's Winds", "Nayru's Loves", "Bombs", "Super Bombs", "Clocks", "Keys", "Magic Containers",
"Triforce Pieces", "Maps", "Compasses", "Boss Keys", "Quivers", "Level Keys", "Canes of Byrna",
"Rupees", "Arrow Ammo", "Fairies", "Magic", "Hearts", "Heart Containers", "Heart Pieces", "Kill All Enemies",
"Bomb Ammo", "Bomb Bags"
};

const char *familylist(int index, int *list_size);

int doInit(zinitdata *zinit)
{
	for(int i=0; i<MAXITEMS; i++)
	{
		int family = itemsbuf[i].family;
		if(family == 0xFF || itemsbuf[i].set_gamedata == 0)
			continue;
		map<int,vector<Family> >::iterator it = families.find(family);
		if(it == families.end())
			families[family] = vector<Family>();
		families[family].push_back(Family(family, itemsbuf[i].fam_type,i));
	}
	//family map has been populated, now sort
	for(map<int, vector<Family> >::iterator it = families.begin(); it != families.end(); it++)
		sort(it->second.begin(), it->second.end());


	init_dlg[0].dp2=lfont;

	init_dlg[5].proc = jwin_initlist_proc;
	init_dlg[5].dp = (void *)familylist;
	init_dlg[5].d1 = -1;

	zinitdata tempdata;
	memcpy(&tempdata, zinit,sizeof(zinitdata));

	int oldselection;
	pair<int *, zinitdata *> p(&oldselection,&tempdata);
	init_dlg[5].dp3 = &p;
	if(families.size() == 0)
	{
		doFamily(-1, &tempdata);
		oldselection = -1;
	}
	else
	{
		doFamily(families.begin()->second[0].family,&tempdata);
		oldselection = 0;
	}

	//special case: potions :(

	for(int i=0; i<3; i++)
	{
		init_dlg[i+77].flags = 0;
	}

	int id = getHighestLevelOfFamily(zinit,itemsbuf,itype_potion);
	if(id == -1)
		init_dlg[77].flags = D_SELECTED;
	else
	{
		int lvl = itemsbuf[id].fam_type;
		if(lvl > 0 && lvl < 3)
			init_dlg[77+lvl].flags = D_SELECTED;
		else
			init_dlg[77].flags = D_SELECTED;
	}

	// items
	char bombstring[5];
	char maxbombstring[5];
	char sbombstring[5];
	char maxsbombstring[5];
	char arrowstring[5];
	char maxarrowstring[5];
	sprintf(bombstring, "%d", zinit->bombs);
	sprintf(maxbombstring, "%d", zinit->max_bombs);
	sprintf(sbombstring, "%d", zinit->super_bombs);
	sprintf(maxsbombstring, "%d", zinit->max_bombs/4);
	sprintf(arrowstring, "%d", zinit->arrows);
	sprintf(maxarrowstring, "%d", zinit->max_arrows);

	init_dlg[109].dp=bombstring;
	init_dlg[110].dp=maxbombstring;
	init_dlg[115].dp=sbombstring;
	init_dlg[116].dp=maxsbombstring;
	init_dlg[119].dp=arrowstring;
	init_dlg[120].dp=maxarrowstring;

	// dmap items

	char key_list[256][4];
	for(int i=0; i<256; i++)
	{
		init_dlg[i+631].flags  = get_bit(zinit->map,i) ? D_SELECTED : 0;
		init_dlg[i+887].flags  = get_bit(zinit->compass,i) ? D_SELECTED : 0;
		init_dlg[i+1143].flags = get_bit(zinit->boss_key,i) ? D_SELECTED : 0;
		sprintf(key_list[i], "%d", zinit->level_keys[i]);
		//sprintf(key_list[i], "%d", 0);
		init_dlg[i+1399].dp = key_list[i];
	}

	// misc
	char tempbuf[5];
	char hcstring[5];
	char sheartstring[5];
	char cheartstring[5];
	char keystring[5];
	char rupiestring[5];
	char magicstring[5];
	char maxmagicstring[5];
	char maxrupeestring[8];
	char maxkeystring[8];

	sprintf(tempbuf, "0");
	sprintf(hcstring, "%d", zinit->hc);
	sprintf(sheartstring, "%d", zinit->start_heart);
	sprintf(cheartstring, "%d", zinit->cont_heart);
	sprintf(keystring, "%d", zinit->keys);
	sprintf(rupiestring, "%d", zinit->rupies);
	sprintf(magicstring, "%d", zinit->magic);
	sprintf(maxmagicstring, "%d", zinit->max_magic);

	init_dlg[1662].dp=hcstring;
	init_dlg[1663].dp=sheartstring;
	init_dlg[1664].dp=cheartstring;
	init_dlg[1665].dp=keystring;
	init_dlg[1666].dp=rupiestring;
	init_dlg[1667].flags = get_bit(zinit->misc,idM_CONTPERCENT) ? D_SELECTED : 0;

	for (int i=0; i<4; i++)
	{
		init_dlg[i+1670].flags=0;
	}
	init_dlg[zinit->hcp+1670].flags=D_SELECTED;

	for(int i=0; i<8; i++)
	{
		init_dlg[1676+i].flags = get_bit(&zinit->triforce,i) ? D_SELECTED : 0;
	}


	init_dlg[1684].flags = get_bit(zinit->misc,idM_CANSLASH) ? D_SELECTED : 0;
	init_dlg[1689].dp=magicstring;
	init_dlg[1690].dp=maxmagicstring;
	init_dlg[1691].flags = get_bit(zinit->misc,idM_DOUBLEMAGIC) ? D_SELECTED : 0;

	sprintf(maxrupeestring, "%d", zinit->max_rupees);
	sprintf(maxkeystring, "%d", zinit->max_keys);


	init_dlg[1695].dp=maxrupeestring;
	init_dlg[1697].dp=maxkeystring;

	int ret = zc_popup_dialog(init_dlg,1);
	if (ret==2)
	{

		//save old selection
		if(oldselection != -1)
		{
			map<int, vector<Family> >::iterator it = families.begin();
			for(int i=0; i<oldselection;i++)
				it++;
			vector<Family> &f = it->second;
			vector<Family>::iterator it2 = f.begin();
			for(int j=7; it2 != f.end() && j<endEquipField; it2++,j++)
			{
				tempdata.items[it2->itemid] = (init_dlg[j].flags == D_SELECTED);
			}
		}
		memcpy(zinit, &tempdata, sizeof(zinitdata));
		zinit->bombs=atoi(bombstring);
		zinit->max_bombs=atoi(maxbombstring);
		zinit->super_bombs=atoi(sbombstring);
		zinit->arrows=atoi(arrowstring);
		zinit->max_arrows=atoi(maxarrowstring);

		// dmap items
		for(int i=0; i<256; i++)
		{
			set_bit(zinit->map,i,init_dlg[i+631].flags);
			set_bit(zinit->compass,i,init_dlg[i+887].flags);
			set_bit(zinit->boss_key,i,init_dlg[i+1143].flags);
			int numkeys=atoi(key_list[i]);
			if(numkeys>255) numkeys=255;
			if(numkeys<0) numkeys=0;
			zinit->level_keys[i]=numkeys;
		}


		// misc
		zinit->start_dmap = init_dlg[1656].d1;
		zinit->hc = min(atoi(hcstring),get_bit(quest_rules,qr_24HC)?24:16);
		zinit->start_heart = min(atoi(sheartstring),zinit->hc);
		set_bit(zinit->misc,idM_CONTPERCENT,init_dlg[1667].flags);
		if (get_bit(zinit->misc,idM_CONTPERCENT))
		{
			zinit->cont_heart = min(atoi(cheartstring),100);
		}
		else
		{
			zinit->cont_heart = min(atoi(cheartstring),zinit->hc);
		}
		zinit->keys = atoi(keystring);
		zinit->rupies = atoi(rupiestring);

		zinit->hcp=0;
		for(int i=0; i<4; i++)
		{
			if(init_dlg[i+1670].flags & D_SELECTED)
			{
				zinit->hcp=i;
			}
		}

		//potions
		removeItemsOfFamily(zinit,itemsbuf,itype_potion);
		for(int i=1; i<3; i++)
		{
			if(init_dlg[i+77].flags & D_SELECTED)
			{
				int potionID = getItemID(itemsbuf, itype_potion,i);
				if(potionID != -1)
					zinit->items[potionID] = true;
			}
		}

		// triforce
		for(int i=0; i<8; i++)
		{
			set_bit(&zinit->triforce,i,init_dlg[1676+i].flags);
		}


		set_bit(zinit->misc,idM_CANSLASH,init_dlg[1684].flags);
		zinit->max_magic = min(atoi(maxmagicstring),8);
		zinit->magic = min(atoi(magicstring),zinit->max_magic);
		set_bit(zinit->misc,idM_DOUBLEMAGIC,init_dlg[1691].flags);
		zinit->max_rupees = vbound(atoi(maxrupeestring), 0, 0xFFFF);
		zinit->max_keys = vbound(atoi(maxkeystring), 0, 0xFFFF);
		onInitOK();
	}
	for(map<int, char *>::iterator it = famnames.begin(); it != famnames.end(); it++)
		delete[] it->second;
	famnames.clear();
	families.clear();
	return D_O_K;
}


const char *familylist(int index, int *list_size)
{
  if(index<0)
  {
	*list_size = families.size();
    return NULL;
  }
  map<int, vector<Family> >::iterator it = families.begin();
  for(int i=0; i<index;i++)
	  it++;
  int family = it->second[0].family;
  //int family = families[index][0].family;
  if(family < itype_last)
  {
	const char *name = itype_names[family];
	return name;
  }
  map<int, char *>::iterator it2 = famnames.find(family);
  if(it2 == famnames.end())
  {
	  char *name = new char[100];
	  sprintf(name, "(New Family %d)", family);
	  famnames[family]=name;
  }
  return famnames[family];
}

void doFamily(int family, zinitdata *data)
{
	if(family == -1)
	{
		for(int i=7; i<endEquipField; i++)
		{
			init_dlg[i].proc = d_dummy_proc;
		}
		return;
	}
	vector<Family> &f = families[family];
	vector<Family>::iterator it = f.begin();
	int i;
	for(i=7; i < endEquipField && it != f.end(); i++, it++)
	{
		init_dlg[i].proc = jwin_checkfont_proc;
		init_dlg[i].dp2 = pfont;
		init_dlg[i].dp = (void *)item_string[it->itemid];
		init_dlg[i].flags = data->items[it->itemid] ? D_SELECTED : 0;
	}
	for(; i<endEquipField; i++)
		init_dlg[i].proc = d_dummy_proc;
}

int jwin_initlist_proc(int msg,DIALOG *d,int c)
{
	int rval = jwin_abclist_proc(msg, d, c);
	pair<int *, zinitdata *> *p = (pair<int *, zinitdata *> *)(d->dp3);
	int *oldselection = p->first;
	if(*oldselection != d->d1 && d->d1 != -1)
	{
		//save old selection
		if(*oldselection != -1)
		{
			map<int, vector<Family> >::iterator it = families.begin();
			for(int i=0; i<*oldselection;i++)
				it++;
			vector<Family> &f = it->second;
			vector<Family>::iterator it2 = f.begin();
			for(int j=7; it2 != f.end() && j<endEquipField; it2++,j++)
			{
				p->second->items[it2->itemid] = (init_dlg[j].flags == D_SELECTED);
			}
		}
		*(p->first) = d->d1;
		map<int, vector<Family> >::iterator it = families.begin();
		for(int i=0; i<d->d1;i++)
			it++;
		doFamily(it->second[0].family, p->second);
		scare_mouse();
		broadcast_dialog_message(MSG_DRAW, 0);
		unscare_mouse();
	}
	return rval;
}

