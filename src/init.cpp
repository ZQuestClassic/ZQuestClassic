#include <map>
#include <vector>
#include <algorithm>
#include <string>

#include <stdio.h>

#include "base/zc_alleg.h"
#include "gui/jwin.h"
#include "base/zdefs.h"
#include "base/dmap.h"
#include "base/zsys.h"
#include "base/gui.h"
#include "init.h"
#include "items.h"
#include "base/qrs.h"
#include "zc/zelda.h"
#include "zinfo.h"
#include <fmt/format.h>
#include <fmt/ranges.h>



#ifdef _MSC_VER
#define stricmp _stricmp
#endif

#define stringizehelp(L) #L
#define stringize(L) stringizehelp(L)

#define vc(x)  ((x)+224)                                    // offset to 'VGA color' x (row 14)

extern int32_t jwin_pal[jcMAX];
extern itemdata *itemsbuf;
extern char *item_string[];
extern byte *colordata;

void initPopulate(int32_t &i, DIALOG_PROC proc, int32_t x, int32_t y, int32_t w, int32_t h, int32_t fg, int32_t bg, int32_t key, int32_t flags, int32_t d1, int32_t d2,
                  void *dp, void *dp2 = NULL, void *dp3 = NULL);
void getitem(int32_t id, bool nosound, bool doRunPassive);

void doFamily(int32_t family, zinitdata *data);

class Family
{
public:
    Family(int32_t fam, int32_t lvl, int32_t id) : family(fam), level(lvl), itemid(id) {}
    int32_t family;
    int32_t level;
    int32_t itemid;
    bool operator<(const Family &other) const
    {
        return level < other.level;
    }
};

extern const char *dmaplist(int32_t index, int32_t *list_size);
extern int32_t onHelp();
extern int32_t startdmapxy[6];
extern void onInitOK();

int32_t d_line_proc(int32_t msg, DIALOG *d, int32_t c)
{
    //these are here to bypass compiler warnings about unused arguments
    c=c;
    
    if(msg==MSG_DRAW)
    {
        int32_t fg = (d->flags & D_DISABLED) ? scheme[jcDISABLED_FG] : d->fg;
        line(screen, d->x, d->y, d->x+d->w, d->y+d->h, fg);
    }
    
    return D_O_K;
}

static int32_t init_equipment_list[] =
{
    // dialog control number
    5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 
	41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, -1
};

static int32_t init_items_list[] =
{
    // dialog control number
    65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 
	100, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 
	128, 129, 130, 131, 132, 133, 134, 135, 136, 1692, 1693, 1698, 1699, -1
};

static int32_t init_scrcnt_tb_list[] =
{
	102, -1
};

static int32_t init_scrcnt_list[] =
{
	// dialog control number
	3276,3277,3278,3279,
	3280,3281,3282,3283,
	3284,3285,3286,3287,
	3288,3289,3290,3291,
	3292,3293,3294,3295,
	
	3296,3297,3298,3299,
	3300,3301,3302,3303,
	3304,3305,3306,3307,
	3308,3309,3310,3311,
	3312,3313,3314,3315,
	
	3316,3317,3318,3319,
	3320,3321,3322,3323,
	3324,3325,3326,3327,
	3328,3329,3330,3331,
	3332,3333,3334,3335,
	
	3336,3337,3338,3339,
	3340,3341,3342,3343,
	3344,3345,3346,3347,
	3348,3349,3350,3351,
	3352,3353,3354,3355,
	
	-1
};

static int32_t init_scrcnt2_list[] =
{
	// dialog control number
	3356,3357,3358,3359,
	3360,3361,3362,3363,
	3364,3365,3366,3367,
	3368,3369,3370,3371,
	3372,3373,3374,3375,
	
	-1
};

static int32_t init_dmap_items_list[] =
{
    // dialog control number
    137, -1
};

static int32_t init_dmap_items_000s_list[] =
{
    // dialog control number
    138, -1
};

static int32_t init_dmap_items_100s_list[] =
{
    // dialog control number
    139, -1
};

static int32_t init_dmap_items_200s_list[] =
{
    // dialog control number
    140, -1
};

static int32_t init_dmap_items_300s_list[] =
{
    // dialog control number
    1707, -1
};

static int32_t init_dmap_items_400s_list[] =
{
    // dialog control number
    1708, -1
};

static int32_t init_dmap_items_500s_list[] =
{
    // dialog control number
    1709, -1
};

static int32_t init_dmap_items_0_00s_list[] =
{
    // dialog control number
    141, 167, 168, 219, 220, 271, 272, 323, 324, 375, 376, 377, 378, 379, 380, 381, 382, 383, 384, 631, 632, 633, 634, 635, 636, 637, 638, 639, 640, 887, 888, 889, 890, 891, 892, 893, 894, 895, 896, 1143, 1144, 1145, 1146, 1147, 1148, 1149, 1150, 1151, 1152, 1399, 1400, 1401, 1402, 1403, 1404, 1405, 1406, 1407, 1408, -1
};

static int32_t init_dmap_items_0_10s_list[] =
{
    // dialog control number
    142, 169, 170, 221, 222, 273, 274, 325, 326, 385, 386, 387, 388, 389, 390, 391, 392, 393, 394, 641, 642, 643, 644, 645, 646, 647, 648, 649, 650, 897, 898, 899, 900, 901, 902, 903, 904, 905, 906, 1153, 1154, 1155, 1156, 1157, 1158, 1159, 1160, 1161, 1162, 1409, 1410, 1411, 1412, 1413, 1414, 1415, 1416, 1417, 1418, -1
};

static int32_t init_dmap_items_0_20s_list[] =
{
    // dialog control number
    143, 171, 172, 223, 224, 275, 276, 327, 328, 395, 396, 397, 398, 399, 400, 401, 402, 403, 404, 651, 652, 653, 654, 655, 656, 657, 658, 659, 660, 907, 908, 909, 910, 911, 912, 913, 914, 915, 916, 1163, 1164, 1165, 1166, 1167, 1168, 1169, 1170, 1171, 1172, 1419, 1420, 1421, 1422, 1423, 1424, 1425, 1426, 1427, 1428, -1
};

static int32_t init_dmap_items_0_30s_list[] =
{
    // dialog control number
    144, 173, 174, 225, 226, 277, 278, 329, 330, 405, 406, 407, 408, 409, 410, 411, 412, 413, 414, 661, 662, 663, 664, 665, 666, 667, 668, 669, 670, 917, 918, 919, 920, 921, 922, 923, 924, 925, 926, 1173, 1174, 1175, 1176, 1177, 1178, 1179, 1180, 1181, 1182, 1429, 1430, 1431, 1432, 1433, 1434, 1435, 1436, 1437, 1438, -1
};

static int32_t init_dmap_items_0_40s_list[] =
{
    // dialog control number
    145, 175, 176, 227, 228, 279, 280, 331, 332, 415, 416, 417, 418, 419, 420, 421, 422, 423, 424, 671, 672, 673, 674, 675, 676, 677, 678, 679, 680, 927, 928, 929, 930, 931, 932, 933, 934, 935, 936, 1183, 1184, 1185, 1186, 1187, 1188, 1189, 1190, 1191, 1192, 1439, 1440, 1441, 1442, 1443, 1444, 1445, 1446, 1447, 1448, -1
};

static int32_t init_dmap_items_0_50s_list[] =
{
    // dialog control number
    146, 177, 178, 229, 230, 281, 282, 333, 334, 425, 426, 427, 428, 429, 430, 431, 432, 433, 434, 681, 682, 683, 684, 685, 686, 687, 688, 689, 690, 937, 938, 939, 940, 941, 942, 943, 944, 945, 946, 1193, 1194, 1195, 1196, 1197, 1198, 1199, 1200, 1201, 1202, 1449, 1450, 1451, 1452, 1453, 1454, 1455, 1456, 1457, 1458, -1
};

static int32_t init_dmap_items_0_60s_list[] =
{
    // dialog control number
    147, 179, 180, 231, 232, 283, 284, 335, 336, 435, 436, 437, 438, 439, 440, 441, 442, 443, 444, 691, 692, 693, 694, 695, 696, 697, 698, 699, 700, 947, 948, 949, 950, 951, 952, 953, 954, 955, 956, 1203, 1204, 1205, 1206, 1207, 1208, 1209, 1210, 1211, 1212, 1459, 1460, 1461, 1462, 1463, 1464, 1465, 1466, 1467, 1468, -1
};

static int32_t init_dmap_items_0_70s_list[] =
{
    // dialog control number
    148, 181, 182, 233, 234, 285, 286, 337, 338, 445, 446, 447, 448, 449, 450, 451, 452, 453, 454, 701, 702, 703, 704, 705, 706, 707, 708, 709, 710, 957, 958, 959, 960, 961, 962, 963, 964, 965, 966, 1213, 1214, 1215, 1216, 1217, 1218, 1219, 1220, 1221, 1222, 1469, 1470, 1471, 1472, 1473, 1474, 1475, 1476, 1477, 1478, -1
};

static int32_t init_dmap_items_0_80s_list[] =
{
    // dialog control number
    149, 183, 184, 235, 236, 287, 288, 339, 340, 455, 456, 457, 458, 459, 460, 461, 462, 463, 464, 711, 712, 713, 714, 715, 716, 717, 718, 719, 720, 967, 968, 969, 970, 971, 972, 973, 974, 975, 976, 1223, 1224, 1225, 1226, 1227, 1228, 1229, 1230, 1231, 1232, 1479, 1480, 1481, 1482, 1483, 1484, 1485, 1486, 1487, 1488, -1
};

static int32_t init_dmap_items_0_90s_list[] =
{
    // dialog control number
    150, 185, 186, 237, 238, 289, 290, 341, 342, 465, 466, 467, 468, 469, 470, 471, 472, 473, 474, 721, 722, 723, 724, 725, 726, 727, 728, 729, 730, 977, 978, 979, 980, 981, 982, 983, 984, 985, 986, 1233, 1234, 1235, 1236, 1237, 1238, 1239, 1240, 1241, 1242, 1489, 1490, 1491, 1492, 1493, 1494, 1495, 1496, 1497, 1498, -1
};

static int32_t init_dmap_items_1_00s_list[] =
{
    // dialog control number
    151, 187, 188, 239, 240, 291, 292, 343, 344, 475, 476, 477, 478, 479, 480, 481, 482, 483, 484, 731, 732, 733, 734, 735, 736, 737, 738, 739, 740, 987, 988, 989, 990, 991, 992, 993, 994, 995, 996, 1243, 1244, 1245, 1246, 1247, 1248, 1249, 1250, 1251, 1252, 1499, 1500, 1501, 1502, 1503, 1504, 1505, 1506, 1507, 1508, -1
};

static int32_t init_dmap_items_1_10s_list[] =
{
    // dialog control number
    152, 189, 190, 241, 242, 293, 294, 345, 346, 485, 486, 487, 488, 489, 490, 491, 492, 493, 494, 741, 742, 743, 744, 745, 746, 747, 748, 749, 750, 997, 998, 999, 1000, 1001, 1002, 1003, 1004, 1005, 1006, 1253, 1254, 1255, 1256, 1257, 1258, 1259, 1260, 1261, 1262, 1509, 1510, 1511, 1512, 1513, 1514, 1515, 1516, 1517, 1518, -1
};

static int32_t init_dmap_items_1_20s_list[] =
{
    // dialog control number
    153, 191, 192, 243, 244, 295, 296, 347, 348, 495, 496, 497, 498, 499, 500, 501, 502, 503, 504, 751, 752, 753, 754, 755, 756, 757, 758, 759, 760, 1007, 1008, 1009, 1010, 1011, 1012, 1013, 1014, 1015, 1016, 1263, 1264, 1265, 1266, 1267, 1268, 1269, 1270, 1271, 1272, 1519, 1520, 1521, 1522, 1523, 1524, 1525, 1526, 1527, 1528, -1
};

static int32_t init_dmap_items_1_30s_list[] =
{
    // dialog control number
    154, 193, 194, 245, 246, 297, 298, 349, 350, 505, 506, 507, 508, 509, 510, 511, 512, 513, 514, 761, 762, 763, 764, 765, 766, 767, 768, 769, 770, 1017, 1018, 1019, 1020, 1021, 1022, 1023, 1024, 1025, 1026, 1273, 1274, 1275, 1276, 1277, 1278, 1279, 1280, 1281, 1282, 1529, 1530, 1531, 1532, 1533, 1534, 1535, 1536, 1537, 1538, -1
};

static int32_t init_dmap_items_1_40s_list[] =
{
    // dialog control number
    155, 195, 196, 247, 248, 299, 300, 351, 352, 515, 516, 517, 518, 519, 520, 521, 522, 523, 524, 771, 772, 773, 774, 775, 776, 777, 778, 779, 780, 1027, 1028, 1029, 1030, 1031, 1032, 1033, 1034, 1035, 1036, 1283, 1284, 1285, 1286, 1287, 1288, 1289, 1290, 1291, 1292, 1539, 1540, 1541, 1542, 1543, 1544, 1545, 1546, 1547, 1548, -1
};

static int32_t init_dmap_items_1_50s_list[] =
{
    // dialog control number
    156, 197, 198, 249, 250, 301, 302, 353, 354, 525, 526, 527, 528, 529, 530, 531, 532, 533, 534, 781, 782, 783, 784, 785, 786, 787, 788, 789, 790, 1037, 1038, 1039, 1040, 1041, 1042, 1043, 1044, 1045, 1046, 1293, 1294, 1295, 1296, 1297, 1298, 1299, 1300, 1301, 1302, 1549, 1550, 1551, 1552, 1553, 1554, 1555, 1556, 1557, 1558, -1
};

static int32_t init_dmap_items_1_60s_list[] =
{
    // dialog control number
    157, 199, 200, 251, 252, 303, 304, 355, 356, 535, 536, 537, 538, 539, 540, 541, 542, 543, 544, 791, 792, 793, 794, 795, 796, 797, 798, 799, 800, 1047, 1048, 1049, 1050, 1051, 1052, 1053, 1054, 1055, 1056, 1303, 1304, 1305, 1306, 1307, 1308, 1309, 1310, 1311, 1312, 1559, 1560, 1561, 1562, 1563, 1564, 1565, 1566, 1567, 1568, -1
};

static int32_t init_dmap_items_1_70s_list[] =
{
    // dialog control number
    158, 201, 202, 253, 254, 305, 306, 357, 358, 545, 546, 547, 548, 549, 550, 551, 552, 553, 554, 801, 802, 803, 804, 805, 806, 807, 808, 809, 810, 1057, 1058, 1059, 1060, 1061, 1062, 1063, 1064, 1065, 1066, 1313, 1314, 1315, 1316, 1317, 1318, 1319, 1320, 1321, 1322, 1569, 1570, 1571, 1572, 1573, 1574, 1575, 1576, 1577, 1578, -1
};

static int32_t init_dmap_items_1_80s_list[] =
{
    // dialog control number
    159, 203, 204, 255, 256, 307, 308, 359, 360, 555, 556, 557, 558, 559, 560, 561, 562, 563, 564, 811, 812, 813, 814, 815, 816, 817, 818, 819, 820, 1067, 1068, 1069, 1070, 1071, 1072, 1073, 1074, 1075, 1076, 1323, 1324, 1325, 1326, 1327, 1328, 1329, 1330, 1331, 1332, 1579, 1580, 1581, 1582, 1583, 1584, 1585, 1586, 1587, 1588, -1
};

static int32_t init_dmap_items_1_90s_list[] =
{
    // dialog control number
    160, 205, 206, 257, 258, 309, 310, 361, 362, 565, 566, 567, 568, 569, 570, 571, 572, 573, 574, 821, 822, 823, 824, 825, 826, 827, 828, 829, 830, 1077, 1078, 1079, 1080, 1081, 1082, 1083, 1084, 1085, 1086, 1333, 1334, 1335, 1336, 1337, 1338, 1339, 1340, 1341, 1342, 1589, 1590, 1591, 1592, 1593, 1594, 1595, 1596, 1597, 1598, -1
};

static int32_t init_dmap_items_2_00s_list[] =
{
    // dialog control number
    161, 207, 208, 259, 260, 311, 312, 363, 364, 575, 576, 577, 578, 579, 580, 581, 582, 583, 584, 831, 832, 833, 834, 835, 836, 837, 838, 839, 840, 1087, 1088, 1089, 1090, 1091, 1092, 1093, 1094, 1095, 1096, 1343, 1344, 1345, 1346, 1347, 1348, 1349, 1350, 1351, 1352, 1599, 1600, 1601, 1602, 1603, 1604, 1605, 1606, 1607, 1608, -1
};

static int32_t init_dmap_items_2_10s_list[] =
{
    // dialog control number
    162, 209, 210, 261, 262, 313, 314, 365, 366, 585, 586, 587, 588, 589, 590, 591, 592, 593, 594, 841, 842, 843, 844, 845, 846, 847, 848, 849, 850, 1097, 1098, 1099, 1100, 1101, 1102, 1103, 1104, 1105, 1106, 1353, 1354, 1355, 1356, 1357, 1358, 1359, 1360, 1361, 1362, 1609, 1610, 1611, 1612, 1613, 1614, 1615, 1616, 1617, 1618, -1
};

static int32_t init_dmap_items_2_20s_list[] =
{
    // dialog control number
    163, 211, 212, 263, 264, 315, 316, 367, 368, 595, 596, 597, 598, 599, 600, 601, 602, 603, 604, 851, 852, 853, 854, 855, 856, 857, 858, 859, 860, 1107, 1108, 1109, 1110, 1111, 1112, 1113, 1114, 1115, 1116, 1363, 1364, 1365, 1366, 1367, 1368, 1369, 1370, 1371, 1372, 1619, 1620, 1621, 1622, 1623, 1624, 1625, 1626, 1627, 1628, -1
};

static int32_t init_dmap_items_2_30s_list[] =
{
    // dialog control number
    164, 213, 214, 265, 266, 317, 318, 369, 370, 605, 606, 607, 608, 609, 610, 611, 612, 613, 614, 861, 862, 863, 864, 865, 866, 867, 868, 869, 870, 1117, 1118, 1119, 1120, 1121, 1122, 1123, 1124, 1125, 1126, 1373, 1374, 1375, 1376, 1377, 1378, 1379, 1380, 1381, 1382, 1629, 1630, 1631, 1632, 1633, 1634, 1635, 1636, 1637, 1638, -1
};

static int32_t init_dmap_items_2_40s_list[] =
{
    // dialog control number
    165, 215, 216, 267, 268, 319, 320, 371, 372, 615, 616, 617, 618, 619, 620, 621, 622, 623, 624, 871, 872, 873, 874, 875, 876, 877, 878, 879, 880, 1127, 1128, 1129, 1130, 1131, 1132, 1133, 1134, 1135, 1136, 1383, 1384, 1385, 1386, 1387, 1388, 1389, 1390, 1391, 1392, 1639, 1640, 1641, 1642, 1643, 1644, 1645, 1646, 1647, 1648, -1
};

static int32_t init_dmap_items_2_50s_list[] =
{
    // dialog control number
    166, 217, 218, 269, 270, 321, 322, 373, 374, 625, 626, 627, 628, 629, 630, 881, 882, 883, 884, 885, 886, 1137, 1138, 1139, 1140, 1141, 1142, 1393, 1394, 1395, 1396, 1397, 1398, 1649, 1650, 1651, 1652, 1653, 1654,
    1984, 1985, 1986, 1987,
    2240, 2241, 2242, 2243,
    2496, 2497, 2498, 2499,
    2752, 2753, 2754, 2755,
    3008, 3009, 3010, 3011, -1
};

static int32_t init_dmap_items_2_60s_list[] =
{
    1750,
    1776, 1777,
    1828, 1829,
    1880, 1881,
    1932, 1933,
    1988, 1989, 1990, 1991, 1992, 1993, 1994, 1995, 1996, 1997,
    2244, 2245, 2246, 2247, 2248, 2249, 2250, 2251, 2252, 2253,
    2500, 2501, 2502, 2503, 2504, 2505, 2506, 2507, 2508, 2509,
    2756, 2757, 2758, 2759, 2760, 2761, 2762, 2763, 2764, 2765,
    3012, 3013, 3014, 3015, 3016, 3017, 3018, 3019, 3020, 3021, -1
};

static int32_t init_dmap_items_2_70s_list[] =
{
    1751,
    1778, 1779,
    1830, 1831,
    1882, 1883,
    1934, 1935,
    1998, 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007,
    2254, 2255, 2256, 2257, 2258, 2259, 2260, 2261, 2262, 2263,
    2510, 2511, 2512, 2513, 2514, 2515, 2516, 2517, 2518, 2519,
    2766, 2767, 2768, 2769, 2770, 2771, 2772, 2773, 2774, 2775,
    3022, 3023, 3024, 3025, 3026, 3027, 3028, 3029, 3030, 3031, -1
};

static int32_t init_dmap_items_2_80s_list[] =
{
    1752,
    1780, 1781,
    1832, 1833,
    1884, 1885,
    1936, 1937,
    2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017,
    2264, 2265, 2266, 2267, 2268, 2269, 2270, 2271, 2272, 2273,
    2520, 2521, 2522, 2523, 2524, 2525, 2526, 2527, 2528, 2529,
    2776, 2777, 2778, 2779, 2780, 2781, 2782, 2783, 2784, 2785,
    3032, 3033, 3034, 3035, 3036, 3037, 3038, 3039, 3040, 3041, -1
};

static int32_t init_dmap_items_2_90s_list[] =
{
    1753,
    1782, 1783,
    1834, 1835,
    1886, 1887,
    1938, 1939,
    2018, 2019, 2020, 2021, 2022, 2023, 2024, 2025, 2026, 2027,
    2274, 2275, 2276, 2277, 2278, 2279, 2280, 2281, 2282, 2283,
    2530, 2531, 2532, 2533, 2534, 2535, 2536, 2537, 2538, 2539,
    2786, 2787, 2788, 2789, 2790, 2791, 2792, 2793, 2794, 2795,
    3042, 3043, 3044, 3045, 3046, 3047, 3048, 3049, 3050, 3051, -1
};

static int32_t init_dmap_items_3_00s_list[] =
{
    1754,
    1784, 1785,
    1836, 1837,
    1888, 1889,
    1940, 1941,
    2028, 2029, 2030, 2031, 2032, 2033, 2034, 2035, 2036, 2037,
    2284, 2285, 2286, 2287, 2288, 2289, 2290, 2291, 2292, 2293,
    2540, 2541, 2542, 2543, 2544, 2545, 2546, 2547, 2548, 2549,
    2796, 2797, 2798, 2799, 2800, 2801, 2802, 2803, 2804, 2805,
    3052, 3053, 3054, 3055, 3056, 3057, 3058, 3059, 3060, 3061, -1
};

static int32_t init_dmap_items_3_10s_list[] =
{
    1755,
    1786, 1787,
    1838, 1839,
    1890, 1891,
    1942, 1943,
    2038, 2039, 2040, 2041, 2042, 2043, 2044, 2045, 2046, 2047,
    2294, 2295, 2296, 2297, 2298, 2299, 2300, 2301, 2302, 2303,
    2550, 2551, 2552, 2553, 2554, 2555, 2556, 2557, 2558, 2559,
    2806, 2807, 2808, 2809, 2810, 2811, 2812, 2813, 2814, 2815,
    3062, 3063, 3064, 3065, 3066, 3067, 3068, 3069, 3070, 3071, -1
};

static int32_t init_dmap_items_3_20s_list[] =
{
    1756,
    1788, 1789,
    1840, 1841,
    1892, 1893,
    1944, 1945,
    2048, 2049, 2050, 2051, 2052, 2053, 2054, 2055, 2056, 2057,
    2304, 2305, 2306, 2307, 2308, 2309, 2310, 2311, 2312, 2313,
    2560, 2561, 2562, 2563, 2564, 2565, 2566, 2567, 2568, 2569,
    2816, 2817, 2818, 2819, 2820, 2821, 2822, 2823, 2824, 2825,
    3072, 3073, 3074, 3075, 3076, 3077, 3078, 3079, 3080, 3081, -1
};

static int32_t init_dmap_items_3_30s_list[] =
{
    1757,
    1790, 1791,
    1842, 1843,
    1894, 1895,
    1946, 1947,
    2058, 2059, 2060, 2061, 2062, 2063, 2064, 2065, 2066, 2067,
    2314, 2315, 2316, 2317, 2318, 2319, 2320, 2321, 2322, 2323,
    2570, 2571, 2572, 2573, 2574, 2575, 2576, 2577, 2578, 2579,
    2826, 2827, 2828, 2829, 2830, 2831, 2832, 2833, 2834, 2835,
    3082, 3083, 3084, 3085, 3086, 3087, 3088, 3089, 3090, 3091, -1
};

static int32_t init_dmap_items_3_40s_list[] =
{
    1758,
    1792, 1793,
    1844, 1845,
    1896, 1897,
    1948, 1949,
    2068, 2069, 2070, 2071, 2072, 2073, 2074, 2075, 2076, 2077,
    2324, 2325, 2326, 2327, 2328, 2329, 2330, 2331, 2332, 2333,
    2580, 2581, 2582, 2583, 2584, 2585, 2586, 2587, 2588, 2589,
    2836, 2837, 2838, 2839, 2840, 2841, 2842, 2843, 2844, 2845,
    3092, 3093, 3094, 3095, 3096, 3097, 3098, 3099, 3100, 3101, -1
};

static int32_t init_dmap_items_3_50s_list[] =
{
    1759,
    1794, 1795,
    1846, 1847,
    1898, 1899,
    1950, 1951,
    2078, 2079, 2080, 2081, 2082, 2083, 2084, 2085, 2086, 2087,
    2334, 2335, 2336, 2337, 2338, 2339, 2340, 2341, 2342, 2343,
    2590, 2591, 2592, 2593, 2594, 2595, 2596, 2597, 2598, 2599,
    2846, 2847, 2848, 2849, 2850, 2851, 2852, 2853, 2854, 2855,
    3102, 3103, 3104, 3105, 3106, 3107, 3108, 3109, 3110, 3111, -1
};

static int32_t init_dmap_items_3_60s_list[] =
{
    1760,
    1796, 1797,
    1848, 1849,
    1900, 1901,
    1952, 1953,
    2088, 2089, 2090, 2091, 2092, 2093, 2094, 2095, 2096, 2097,
    2344, 2345, 2346, 2347, 2348, 2349, 2350, 2351, 2352, 2353,
    2600, 2601, 2602, 2603, 2604, 2605, 2606, 2607, 2608, 2609,
    2856, 2857, 2858, 2859, 2860, 2861, 2862, 2863, 2864, 2865,
    3112, 3113, 3114, 3115, 3116, 3117, 3118, 3119, 3120, 3121, -1
};

static int32_t init_dmap_items_3_70s_list[] =
{
    1761,
    1798, 1799,
    1850, 1851,
    1902, 1903,
    1954, 1955,
    2098, 2099, 2100, 2101, 2102, 2103, 2104, 2105, 2106, 2107,
    2354, 2355, 2356, 2357, 2358, 2359, 2360, 2361, 2362, 2363,
    2610, 2611, 2612, 2613, 2614, 2615, 2616, 2617, 2618, 2619,
    2866, 2867, 2868, 2869, 2870, 2871, 2872, 2873, 2874, 2875,
    3122, 3123, 3124, 3125, 3126, 3127, 3128, 3129, 3130, 3131, -1
};

static int32_t init_dmap_items_3_80s_list[] =
{
    1762,
    1800, 1801,
    1852, 1853,
    1904, 1905,
    1956, 1957,
    2108, 2109, 2110, 2111, 2112, 2113, 2114, 2115, 2116, 2117,
    2364, 2365, 2366, 2367, 2368, 2369, 2370, 2371, 2372, 2373,
    2620, 2621, 2622, 2623, 2624, 2625, 2626, 2627, 2628, 2629,
    2876, 2877, 2878, 2879, 2880, 2881, 2882, 2883, 2884, 2885,
    3132, 3133, 3134, 3135, 3136, 3137, 3138, 3139, 3140, 3141, -1
};

static int32_t init_dmap_items_3_90s_list[] =
{
    1763,
    1802, 1803,
    1854, 1855,
    1906, 1907,
    1958, 1959,
    2118, 2119, 2120, 2121, 2122, 2123, 2124, 2125, 2126, 2127,
    2374, 2375, 2376, 2377, 2378, 2379, 2380, 2381, 2382, 2383,
    2630, 2631, 2632, 2633, 2634, 2635, 2636, 2637, 2638, 2639,
    2886, 2887, 2888, 2889, 2890, 2891, 2892, 2893, 2894, 2895,
    3142, 3143, 3144, 3145, 3146, 3147, 3148, 3149, 3150, 3151, -1
};

static int32_t init_dmap_items_4_00s_list[] =
{
    1764,
    1804, 1805,
    1856, 1857,
    1908, 1909,
    1960, 1961,
    2128, 2129, 2130, 2131, 2132, 2133, 2134, 2135, 2136, 2137,
    2384, 2385, 2386, 2387, 2388, 2389, 2390, 2391, 2392, 2393,
    2640, 2641, 2642, 2643, 2644, 2645, 2646, 2647, 2648, 2649,
    2896, 2897, 2898, 2899, 2900, 2901, 2902, 2903, 2904, 2905,
    3152, 3153, 3154, 3155, 3156, 3157, 3158, 3159, 3160, 3161, -1
};

static int32_t init_dmap_items_4_10s_list[] =
{
    1765,
    1806, 1807,
    1858, 1859,
    1910, 1911,
    1962, 1963,
    2138, 2139, 2140, 2141, 2142, 2143, 2144, 2145, 2146, 2147,
    2394, 2395, 2396, 2397, 2398, 2399, 2400, 2401, 2402, 2403,
    2650, 2651, 2652, 2653, 2654, 2655, 2656, 2657, 2658, 2659,
    2906, 2907, 2908, 2909, 2910, 2911, 2912, 2913, 2914, 2915,
    3162, 3163, 3164, 3165, 3166, 3167, 3168, 3169, 3170, 3171, -1
};

static int32_t init_dmap_items_4_20s_list[] =
{
    1766,
    1808, 1809,
    1860, 1861,
    1912, 1913,
    1964, 1965,
    2148, 2149, 2150, 2151, 2152, 2153, 2154, 2155, 2156, 2157,
    2404, 2405, 2406, 2407, 2408, 2409, 2410, 2411, 2412, 2413,
    2660, 2661, 2662, 2663, 2664, 2665, 2666, 2667, 2668, 2669,
    2916, 2917, 2918, 2919, 2920, 2921, 2922, 2923, 2924, 2925,
    3172, 3173, 3174, 3175, 3176, 3177, 3178, 3179, 3180, 3181, -1
};

static int32_t init_dmap_items_4_30s_list[] =
{
    1767,
    1810, 1811,
    1862, 1863,
    1914, 1915,
    1966, 1967,
    2158, 2159, 2160, 2161, 2162, 2163, 2164, 2165, 2166, 2167,
    2414, 2415, 2416, 2417, 2418, 2419, 2420, 2421, 2422, 2423,
    2670, 2671, 2672, 2673, 2674, 2675, 2676, 2677, 2678, 2679,
    2926, 2927, 2928, 2929, 2930, 2931, 2932, 2933, 2934, 2935,
    3182, 3183, 3184, 3185, 3186, 3187, 3188, 3189, 3190, 3191, -1
};

static int32_t init_dmap_items_4_40s_list[] =
{
    1768,
    1812, 1813,
    1864, 1865,
    1916, 1917,
    1968, 1969,
    2168, 2169, 2170, 2171, 2172, 2173, 2174, 2175, 2176, 2177,
    2424, 2425, 2426, 2427, 2428, 2429, 2430, 2431, 2432, 2433,
    2680, 2681, 2682, 2683, 2684, 2685, 2686, 2687, 2688, 2689,
    2936, 2937, 2938, 2939, 2940, 2941, 2942, 2943, 2944, 2945,
    3192, 3193, 3194, 3195, 3196, 3197, 3198, 3199, 3200, 3201, -1
};

static int32_t init_dmap_items_4_50s_list[] =
{
    1769,
    1814, 1815,
    1866, 1867,
    1918, 1919,
    1970, 1971,
    2178, 2179, 2180, 2181, 2182, 2183, 2184, 2185, 2186, 2187,
    2434, 2435, 2436, 2437, 2438, 2439, 2440, 2441, 2442, 2443,
    2690, 2691, 2692, 2693, 2694, 2695, 2696, 2697, 2698, 2699,
    2946, 2947, 2948, 2949, 2950, 2951, 2952, 2953, 2954, 2955,
    3202, 3203, 3204, 3205, 3206, 3207, 3208, 3209, 3210, 3211, -1
};

static int32_t init_dmap_items_4_60s_list[] =
{
    1770,
    1816, 1817,
    1868, 1869,
    1920, 1921,
    1972, 1973,
    2188, 2189, 2190, 2191, 2192, 2193, 2194, 2195, 2196, 2197,
    2444, 2445, 2446, 2447, 2448, 2449, 2450, 2451, 2452, 2453,
    2700, 2701, 2702, 2703, 2704, 2705, 2706, 2707, 2708, 2709,
    2956, 2957, 2958, 2959, 2960, 2961, 2962, 2963, 2964, 2965,
    3212, 3213, 3214, 3215, 3216, 3217, 3218, 3219, 3220, 3221, -1
};

static int32_t init_dmap_items_4_70s_list[] =
{
    1771,
    1818, 1819,
    1870, 1871,
    1922, 1923,
    1974, 1975,
    2198, 2199, 2200, 2201, 2202, 2203, 2204, 2205, 2206, 2207,
    2454, 2455, 2456, 2457, 2458, 2459, 2460, 2461, 2462, 2463,
    2710, 2711, 2712, 2713, 2714, 2715, 2716, 2717, 2718, 2719,
    2966, 2967, 2968, 2969, 2970, 2971, 2972, 2973, 2974, 2975,
    3222, 3223, 3224, 3225, 3226, 3227, 3228, 3229, 3230, 3231, -1
};

static int32_t init_dmap_items_4_80s_list[] =
{
    1772,
    1820, 1821,
    1872, 1873,
    1924, 1925,
    1976, 1977,
    2208, 2209, 2210, 2211, 2212, 2213, 2214, 2215, 2216, 2217,
    2464, 2465, 2466, 2467, 2468, 2469, 2470, 2471, 2472, 2473,
    2720, 2721, 2722, 2723, 2724, 2725, 2726, 2727, 2728, 2729,
    2976, 2977, 2978, 2979, 2980, 2981, 2982, 2983, 2984, 2985,
    3232, 3233, 3234, 3235, 3236, 3237, 3238, 3239, 3240, 3241, -1
};

static int32_t init_dmap_items_4_90s_list[] =
{
    1773,
    1822, 1823,
    1874, 1875,
    1926, 1927,
    1978, 1979,
    2218, 2219, 2220, 2221, 2222, 2223, 2224, 2225, 2226, 2227,
    2474, 2475, 2476, 2477, 2478, 2479, 2480, 2481, 2482, 2483,
    2730, 2731, 2732, 2733, 2734, 2735, 2736, 2737, 2738, 2739,
    2986, 2987, 2988, 2989, 2990, 2991, 2992, 2993, 2994, 2995,
    3242, 3243, 3244, 3245, 3246, 3247, 3248, 3249, 3250, 3251, -1
};

static int32_t init_dmap_items_5_00s_list[] =
{
    1774,
    1824, 1825,
    1876, 1877,
    1928, 1929,
    1980, 1981,
    2228, 2229, 2230, 2231, 2232, 2233, 2234, 2235, 2236, 2237,
    2484, 2485, 2486, 2487, 2488, 2489, 2490, 2491, 2492, 2493,
    2740, 2741, 2742, 2743, 2744, 2745, 2746, 2747, 2748, 2749,
    2996, 2997, 2998, 2999, 3000, 3001, 3002, 3003, 3004, 3005,
    3252, 3253, 3254, 3255, 3256, 3257, 3258, 3259, 3260, 3261, -1
};

static int32_t init_dmap_items_5_10s_list[] =
{
    1775,
    1826, 1827,
    1878, 1879,
    1930, 1931,
    1982, 1983,
    2238, 2239,
    2494, 2495,
    2750, 2751,
    3006, 3007,
    3262, 3263, -1
};

static int32_t init_misc_list[] =
{
    // dialog control number
    //  1605, 1606, 1607, 1608, 1609, 1610, 1611, 1612, 1613, 1614, 1615, 1616, 1617, 1618, 1619, 1620, 1621, 1622, 1623, 1624, 1625, 1626, 1627, 1628, 1629, 1630, 1631, 1632, 1633, 1634, 1635, 1636, 1637, 1638, 1639, 1640, 1641, -1
    1655, 1656, 1657, 1658, 1659, 1660, 1661, 1662, 1663, 1664, 1665, 1666, 1667, 1668, 1669, 1670, 1671, 1672, 1673, 1674, 1675, 1676, 1677, 1678, 1679, 1680, 1681, 1682, 1683, 1684, 1685, 1686, 1687, 1688, 1689, 1690, 1691, 1694, 1695, 1696, 1697, -1
};

static int32_t init_var_list[] =
{
    1700, 1701, 1702, 1703, 1704, 1705, 1706, 
	3264, 3265, 3266, 3267,
	3268, 3269, 3270, 3271, 3272, 3273, 3274, 3275,
	-1
};

static int32_t init_var_list2[] =
{
	3376, 3377, 3378, 3379, 3380, 3381, 3382, 3383, 3384, 3385, 3386, 3387,
	-1
};

static int32_t init_var_list3[] =
{
	3388, 3389, 3390, 3391, 3392, 3393, 3394, 3395,
	-1
};

static int32_t init_var_tb_list[] =
{
	101, -1
};


static TABPANEL init_dmap_items_hundreds_tabs[] =
{
    // (text)
    { (char *)"000",             D_SELECTED,   init_dmap_items_000s_list,    0, NULL },
    { (char *)"100",             0,            init_dmap_items_100s_list,    0, NULL },
    { (char *)"200",             0,            init_dmap_items_200s_list,    0, NULL },
    { (char *)"300",             0,            init_dmap_items_300s_list,    0, NULL },
    { (char *)"400",             0,            init_dmap_items_400s_list,    0, NULL },
    { (char *)"500",             0,            init_dmap_items_500s_list,    0, NULL },
    { NULL,                      0,            0,                            0, NULL }
};


static TABPANEL init_dmap_items_000s_tabs[] =
{
    // (text)
    { (char *)"00",              D_SELECTED,   init_dmap_items_0_00s_list,   0, NULL },
    { (char *)"10",              0,            init_dmap_items_0_10s_list,   0, NULL },
    { (char *)"20",              0,            init_dmap_items_0_20s_list,   0, NULL },
    { (char *)"30",              0,            init_dmap_items_0_30s_list,   0, NULL },
    { (char *)"40",              0,            init_dmap_items_0_40s_list,   0, NULL },
    { (char *)"50",              0,            init_dmap_items_0_50s_list,   0, NULL },
    { (char *)"60",              0,            init_dmap_items_0_60s_list,   0, NULL },
    { (char *)"70",              0,            init_dmap_items_0_70s_list,   0, NULL },
    { (char *)"80",              0,            init_dmap_items_0_80s_list,   0, NULL },
    { (char *)"90",              0,            init_dmap_items_0_90s_list,   0, NULL },
    { NULL,                      0,            0,                            0, NULL }
};

static TABPANEL init_dmap_items_100s_tabs[] =
{
    // (text)
    { (char *)"00",              D_SELECTED,   init_dmap_items_1_00s_list,   0, NULL },
    { (char *)"10",              0,            init_dmap_items_1_10s_list,   0, NULL },
    { (char *)"20",              0,            init_dmap_items_1_20s_list,   0, NULL },
    { (char *)"30",              0,            init_dmap_items_1_30s_list,   0, NULL },
    { (char *)"40",              0,            init_dmap_items_1_40s_list,   0, NULL },
    { (char *)"50",              0,            init_dmap_items_1_50s_list,   0, NULL },
    { (char *)"60",              0,            init_dmap_items_1_60s_list,   0, NULL },
    { (char *)"70",              0,            init_dmap_items_1_70s_list,   0, NULL },
    { (char *)"80",              0,            init_dmap_items_1_80s_list,   0, NULL },
    { (char *)"90",              0,            init_dmap_items_1_90s_list,   0, NULL },
    { NULL,                      0,            0,                            0, NULL }
};

static TABPANEL init_dmap_items_200s_tabs[] =
{
    // (text)
    { (char *)"00",              D_SELECTED,   init_dmap_items_2_00s_list,   0, NULL },
    { (char *)"10",              0,            init_dmap_items_2_10s_list,   0, NULL },
    { (char *)"20",              0,            init_dmap_items_2_20s_list,   0, NULL },
    { (char *)"30",              0,            init_dmap_items_2_30s_list,   0, NULL },
    { (char *)"40",              0,            init_dmap_items_2_40s_list,   0, NULL },
    { (char *)"50",              0,            init_dmap_items_2_50s_list,   0, NULL },
    { (char *)"60",              0,            init_dmap_items_2_60s_list,   0, NULL },
    { (char *)"70",              0,            init_dmap_items_2_70s_list,   0, NULL },
    { (char *)"80",              0,            init_dmap_items_2_80s_list,   0, NULL },
    { (char *)"90",              0,            init_dmap_items_2_90s_list,   0, NULL },
    { NULL,                      0,            0,                            0, NULL }
};

static TABPANEL init_dmap_items_300s_tabs[] =
{
    // (text)
    { (char *)"00",              D_SELECTED,   init_dmap_items_3_00s_list,   0, NULL },
    { (char *)"10",              0,            init_dmap_items_3_10s_list,   0, NULL },
    { (char *)"20",              0,            init_dmap_items_3_20s_list,   0, NULL },
    { (char *)"30",              0,            init_dmap_items_3_30s_list,   0, NULL },
    { (char *)"40",              0,            init_dmap_items_3_40s_list,   0, NULL },
    { (char *)"50",              0,            init_dmap_items_3_50s_list,   0, NULL },
    { (char *)"60",              0,            init_dmap_items_3_60s_list,   0, NULL },
    { (char *)"70",              0,            init_dmap_items_3_70s_list,   0, NULL },
    { (char *)"80",              0,            init_dmap_items_3_80s_list,   0, NULL },
    { (char *)"90",              0,            init_dmap_items_3_90s_list,   0, NULL },
    { NULL,                      0,            0,                            0, NULL }
};

static TABPANEL init_dmap_items_400s_tabs[] =
{
    // (text)
    { (char *)"00",              D_SELECTED,   init_dmap_items_4_00s_list,   0, NULL },
    { (char *)"10",              0,            init_dmap_items_4_10s_list,   0, NULL },
    { (char *)"20",              0,            init_dmap_items_4_20s_list,   0, NULL },
    { (char *)"30",              0,            init_dmap_items_4_30s_list,   0, NULL },
    { (char *)"40",              0,            init_dmap_items_4_40s_list,   0, NULL },
    { (char *)"50",              0,            init_dmap_items_4_50s_list,   0, NULL },
    { (char *)"60",              0,            init_dmap_items_4_60s_list,   0, NULL },
    { (char *)"70",              0,            init_dmap_items_4_70s_list,   0, NULL },
    { (char *)"80",              0,            init_dmap_items_4_80s_list,   0, NULL },
    { (char *)"90",              0,            init_dmap_items_4_90s_list,   0, NULL },
    { NULL,                      0,            0,                            0, NULL }
};

static TABPANEL init_dmap_items_500s_tabs[] =
{
    // (text)
    { (char *)"00",              D_SELECTED,   init_dmap_items_5_00s_list,   0, NULL },
    { (char *)"10",              0,            init_dmap_items_5_10s_list,   0, NULL },
    { NULL,                      0,            0,                            0, NULL }
};

TABPANEL init_tabs[] =
{
    // (text)
    { (char *)"Equipment",       D_SELECTED,   init_equipment_list,          0, NULL },
    { (char *)"Items",           0,            init_items_list,              0, NULL },
    { (char *)"Script Counters", 0,            init_scrcnt_tb_list,          0, NULL },
    { (char *)"Level Items",     0,            init_dmap_items_list,         0, NULL },
    { (char *)"Misc",            0,            init_misc_list,               0, NULL },
    { (char *)"Variables",       0,            init_var_tb_list,             0, NULL },
    { NULL,                      0,            0,                            0, NULL }
};

TABPANEL init_scrcnt_tabs[] =
{
    // (text)
    { (char *)"1",               D_SELECTED,   init_scrcnt_list,          0, NULL },
    { (char *)"2",               0,            init_scrcnt2_list,         0, NULL },
    { NULL,                      0,            0,                         0, NULL }
};

TABPANEL init_var_tabs[] =
{
    // (text)
    { (char *)"1",               D_SELECTED,   init_var_list,          0, NULL },
    { (char *)"2",               0,            init_var_list2,         0, NULL },
    { (char *)"3",               0,            init_var_list3,         0, NULL },
    { NULL,                      0,            0,                      0, NULL }
};

//int32_t startdmapxy[6] = {188-68,131-93,188-68,111-93,188-68,120-93};

int32_t d_maxbombsedit_proc(int32_t msg,DIALOG *d,int32_t c)
{
    int32_t ret = jwin_edit_proc(msg,d,c);
    
    if(msg==MSG_DRAW)
    {
        int32_t div = atoi((char*)((d+1589)->dp));
        
        if(div == 0)
            div = 4;
            
        sprintf((char*)((d+6)->dp), "%d", atoi((char*)(d->dp))/div);
        (d+6)->proc(MSG_DRAW,d+6,0);
    }
    
    return ret;
}

int32_t d_bombratioedit_proc(int32_t msg,DIALOG *d,int32_t c)
{
    int32_t ret = jwin_edit_proc(msg,d,c);
    
    if(msg==MSG_DRAW)
    {
        int32_t sbombmax = 0;
        int32_t div = atoi((char*)(d->dp));
        
        if(div == 0)
            div = 4;
            
        if(atoi((char*)(d->dp)))
            sbombmax = atoi((char*)((d-1589)->dp))/div;
            
        sprintf((char*)((d-1583)->dp), "%d", sbombmax);
        (d-1583)->proc(MSG_DRAW,d-1583,0);
    }
    
    return ret;
}

enum { ws_2_frame, ws_3_frame, ws_max };
const char *walkstyles[]= { "2-frame", "3-frame" };

char *walkstylelist(int32_t index, int32_t *list_size)
{
    if(index>=0)
    {
        return (char *)walkstyles[index];
    }
    
    *list_size=ws_max;
    return NULL;
}

ListData dmap_list(dmaplist, &font);

//InitData

//Item Class (Itemclass, itemdata family names in ZQuest.)
const char *itype_names[itype_max] = { "Swords", "Boomerangs", "Arrows", "Candles", "Whistles",
                                       "Bait", "Letters", "Potions", "Wands", "Rings", "Wallets", "Amulets", "Shields", "Bows", "Rafts",
                                       "Ladders", "Books", "Magic Keys", "Bracelets", "Flippers", "Boots", "Hookshots", "Lenses", "Hammers",
                                       "Divine Fire", "Divine Escape", "Divine Protection", "Bombs", "Super Bombs", "Clocks", "Keys", "Magic Containers",
                                       "Triforce Pieces", "Maps", "Compasses", "Boss Keys", "Quivers", "Level Keys", "Canes of Byrna", "Rupees", "Arrow Ammo",
                                       "Fairies", "Magic", "Hearts", "Heart Containers", "Heart Pieces", "Kill All Enemies",
                                       "Bomb Ammo", "Bomb Bags", "Roc Items", "Hover Boots", "Scroll: Spin Attack", "Scroll: Cross Beams", "Scroll: Quake Hammer",
                                       "Whisp Rings", "Charge Rings", "Scroll: Peril Beam", "Wealth Medals", "Heart Rings", "Magic Rings", "Scroll: Hurricane Spin", 
					"Scroll: Super Quake",
                                       "Stones of Agony", "Stomp Boots", "Whimsical Rings", "Peril Rings", "Non-gameplay Items",
                                       "zz67", "zz68", "zz69", "zz70", "zz71",
                                       "zz72", "zz73", "zz74", "zz75", "zz76",
                                       "zz77", "zz78", "zz79", "zz80", "zz81",
                                       "zz82", "zz83", "zz84", "zz85", "zz86",
                                       "Bow and Arrow (Subscreen Only)", "Letter or Potion (Subscreen Only)"
                                     };
//New Item Classes dfor 2.54 andd above. 
const char *itype_new_names[itype_max] = { "Script 1","Script 2","Script 3","Script 4","Script 5","Script 6","Script 7","Script 8","Script 9","Script 10", "Ice Rod"
                                     };


#include "dialog/init_data.h"
int32_t doInit(zinitdata *local_zinit, bool isZC)
{
	call_init_dlg(*local_zinit, isZC);
    return D_O_K;
}

// NOTE: This method has been severely hacked to fix an annoying problem at game start:
// items (ie the Small Wallet) which modify max counter values need to be processed after
// the values for those counters specified in init data, as the author expects these items
// to modify the max counter. BUT the counter value should NOT be updated, ie, starting with
// the bomb item does not give 8 free bombs at quest start.
// I don't like this solution one bit, but can't come up with anything better -DD
void resetItems(gamedata *game2, zinitdata *zinit2, bool freshquest)
{
	game2->set_maxlife(zinit2->mcounter[crLIFE]);
	game2->set_maxbombs(zinit2->mcounter[crBOMBS]);
	game2->set_maxcounter(zinit2->mcounter[crBOMBS]/zc_max(1,zinit2->bomb_ratio), crSBOMBS);
	game2->set_maxmagic(zinit2->mcounter[crMAGIC]);
	game2->set_maxarrows(zinit2->mcounter[crARROWS]);
	game2->set_maxcounter(zinit2->mcounter[crMONEY], crMONEY);
	game2->set_maxcounter(zinit2->mcounter[crKEYS], crKEYS);
	
	for(int32_t q = 0; q < 100; ++q)
		game2->set_maxcounter(zinit2->mcounter[q+crCUSTOM1], q+crCUSTOM1);
	
	//set up the items
	for(int32_t i=0; i<MAXITEMS; i++)
	{
		if(zinit2->get_item(i) && (itemsbuf[i].flags & item_gamedata))
		{
#ifndef IS_EDITOR
			if (!game2->get_item(i))
				getitem(i,true,false);
#else
			game2->set_item_no_flush(i, true);
#endif
		}
		else
			game2->set_item_no_flush(i,false);
			
		game2->items_off[i] = 0;
		
		// Fix them DMap items
		// Since resetItems() gets called before AND after init_dmap()...
		if(get_currdmap() > -1)
			game2->items_off[i] |= DMaps[get_currdmap()].disableditems[i];
	}
	
	flushItemCache();
	
	//Then set up the counters
	game2->set_life(zc_max(1,zinit2->counter[crLIFE]));
	game2->set_bombs(zinit2->counter[crBOMBS]);
	
	if(zinit2->counter[crBOMBS] > 0 && zinit2->mcounter[crBOMBS] > 0) game2->set_item(iBombs, true);
	
	game2->set_keys(zinit2->counter[crKEYS]);
	game2->set_sbombs(zinit2->counter[crSBOMBS]);
	
	if(zinit2->counter[crSBOMBS] > 0 && (zinit2->mcounter[crBOMBS] /zc_max(1,zinit2->bomb_ratio)) > 0) game2->set_item(iSBomb, true);
	
	game2->set_HCpieces(zinit2->hcp);
	game2->set_rupies(zinit2->counter[crMONEY]);
	game2->set_hcp_per_hc(zinit2->hcp_per_hc);
	game2->set_cont_hearts(zinit2->cont_heart);
	game2->set_cont_percent(zinit2->flags.get(INIT_FL_CONTPERCENT));
	game2->set_hp_per_heart(zinit2->hp_per_heart);
	game2->set_mp_per_block(zinit2->magic_per_block);
	game2->set_hero_dmgmult(zinit2->hero_damage_multiplier);
	game2->set_regionmapping(zinit2->region_mapping);
	game2->set_item_spawn_flicker(zinit2->item_spawn_flicker);
	game2->set_item_timeout_dur(zinit2->item_timeout_dur);
	game2->set_item_timeout_flicker(zinit2->item_timeout_flicker);
	game2->set_item_flicker_speed(zinit2->item_flicker_speed);
	game2->set_ene_dmgmult(zinit2->ene_damage_multiplier);
	game2->set_dither_type(zinit2->dither_type);
	game2->set_dither_arg(zinit2->dither_arg);
	game2->set_dither_perc(zinit2->dither_percent);
	game2->set_light_rad(zinit2->def_lightrad);
	game2->set_transdark_perc(zinit2->transdark_percent);
	game2->set_darkscr_color(zinit2->darkcol);
	game2->set_light_wave_rate(zinit2->light_wave_rate);
	game2->set_light_wave_size(zinit2->light_wave_size);
	game2->set_watergrav(zinit2->swimgravity);
	game2->set_sideswim_up(zinit2->heroSideswimUpStep);
	game2->set_sideswim_side(zinit2->heroSideswimSideStep);
	game2->set_sideswim_down(zinit2->heroSideswimDownStep);
	game2->set_sideswim_jump(zinit2->exitWaterJump);
	game2->set_bunny_ltm(zinit2->bunny_ltm);
	game2->set_switchhookstyle(zinit2->switchhookstyle);
	game2->set_spriteflickerspeed(zinit2->spriteflickerspeed);
	game2->set_spriteflickercolor(zinit2->spriteflickercolor);
	game2->set_spriteflickertransp(zinit2->spriteflickertransp);
	
	for(int32_t i=0; i<MAXLEVELS; i++)
		game2->lvlitems[i] = zinit2->litems[i];
	game2->lvlswitches = zinit2->lvlswitches;
	game2->lvlkeys = zinit2->level_keys;
	for(uint q = 0; q < NUM_BOTTLE_SLOTS; ++q)
		game2->bottleSlots[q] = zinit2->bottle_slot[q];
	
	game2->set_magic(zc_min(zinit2->counter[crMAGIC],zinit2->mcounter[crMAGIC]));
	game2->set_magicdrainrate(zinit2->magicdrainrate);
	game2->set_canslash(zinit2->flags.get(INIT_FL_CANSLASH)?1:0);
	
	game2->set_arrows(zinit2->counter[crARROWS]);
	
	for(int32_t q = 0; q < 100; ++q)
		game2->set_counter(zinit2->counter[q+crCUSTOM1], q+ crCUSTOM1);
	
	if(freshquest)
	{
		game2->gen_doscript = zinit2->gen_doscript;
		game2->gen_exitState = zinit2->gen_exitState;
		game2->gen_reloadState = zinit2->gen_reloadState;
		game2->gen_initd = zinit2->gen_initd;
		game2->gen_data = zinit2->gen_data;
		game2->gen_eventstate = zinit2->gen_eventstate;
		game2->screen_data = zinit2->screen_data;
	}
	
	game2->swim_mult = zinit2->hero_swim_mult;
	game2->swim_div = zinit2->hero_swim_div;
	game2->normalize();
	//flush the cache again (in case bombs became illegal to use by setting bombs to 0)
	flushItemCache();
}

template<std::size_t N, class T>
constexpr std::size_t countof(T(&)[N]) { return N; }

#define LIST_PROPS \
	ZFIXPROP(air_drag) \
	PROP(bomb_ratio) \
	PROP(bunny_ltm) \
	PROP(cont_heart) \
	PROP(darkcol) \
	PROP(def_lightrad) \
	PROP(dither_arg) \
	PROP(dither_percent) \
	PROP(dither_type) \
	PROP(ene_damage_multiplier) \
	PROP(exitWaterJump) \
	PROP(gravity) \
	PROP(hcp) \
	PROP(hcp_per_hc) \
	PROP(hero_damage_multiplier) \
	PROP(hero_swim_div) \
	PROP(hero_swim_mult) \
	PROP(hero_swim_speed) \
	PROP(heroAnimationStyle) \
	PROP(heroSideswimDownStep) \
	PROP(heroSideswimSideStep) \
	PROP(heroSideswimUpStep) \
	PROP(heroStep) \
	PROP(hp_per_heart) \
	PROP(item_spawn_flicker) \
	PROP(item_timeout_dur) \
	PROP(item_timeout_flicker) \
	PROP(item_flicker_speed) \
	PROP(jump_hero_layer_threshold) \
	PROP(last_map) \
	PROP(last_screen) \
	PROP(light_wave_rate) \
	PROP(light_wave_size) \
	PROP(magic_per_block) \
	PROP(magicdrainrate) \
	PROP(msg_more_is_offset) \
	PROP(msg_more_x) \
	PROP(msg_more_y) \
	PROP(msg_speed) \
	PROP(region_mapping) \
	ZFIXPROP(shove_offset) \
	PROP(ss_bbox_1_color) \
	PROP(ss_bbox_2_color) \
	PROP(ss_flags) \
	PROP(ss_grid_color) \
	PROP(ss_grid_x) \
	PROP(ss_grid_xofs) \
	PROP(ss_grid_y) \
	PROP(ss_grid_yofs) \
	PROP(start_dmap) \
	PROP(subscrSpeed) \
	PROP(swimgravity) \
	PROP(switchhookstyle) \
	PROP(spriteflickerspeed) \
	PROP(spriteflickercolor) \
	PROP(spriteflickertransp) \
	PROP(terminalv) \
	PROP(transdark_percent)

#define LIST_ARRAY_PROPS \
	ARRAY_PROP(bottle_slot) \
	ARRAY_PROP(counter) \
	BITSTR_PROP(gen_doscript) \
	VEC_PROP(gen_eventstate) \
	VEC_PROP(gen_exitState) \
	VEC_PROP(gen_reloadState) \
	VEC_PROP_2D(gen_initd) \
	VEC_PROP_2D(gen_data) \
	ARRAY_PROP(items) \
	VEC_PROP(level_keys) \
	VEC_PROP(lvlswitches) \
	ARRAY_PROP(litems) \
	ARRAY_PROP(mcounter) \
	ARRAY_PROP(sprite_z_thresholds) \
	BITSTR_PROP(flags)

static int DELTA_VERSION = 2;
string serialize_init_data_delta(zinitdata *base, zinitdata *changed)
{
	vector<string> tokens;
	tokens.push_back(fmt::format("VERSION={}",DELTA_VERSION));
	
	#define PROP(name) if (base->name != changed->name) \
		tokens.push_back(fmt::format("{}={}", #name, (int)changed->name));
	#define ZFIXPROP(name) if(base->name != changed->name) \
		tokens.push_back(fmt::format("{}={}", #name, (int)changed->name.getZLong()));
	LIST_PROPS;
	#undef ZFIXPROP
	#undef PROP
	
	#define ARRAY_PROP(name) \
		for (int i = 0; i < countof(base->name); i++) \
			if (base->name[i] != changed->name[i]) \
				tokens.push_back(fmt::format("{}[{}]={}", #name, i, (int)changed->name[i]));
	#define VEC_PROP(name) \
		for (int i = 0; i < base->name.size(); i++) \
			if (base->name[i] != changed->name[i]) \
				tokens.push_back(fmt::format("{}[{}]={}", #name, i, (int)changed->name[i]));
	#define VEC_PROP_2D(name) \
		for (int i = 0; i < base->name.size(); i++) \
			for (int j = 0; j < base->name[i].size(); j++) \
				if (base->name[i][j] != changed->name[i][j]) \
					tokens.push_back(fmt::format("{}[{}][{}]={}", #name, i, j, int(changed->name[i][j])));
	#define BITSTR_PROP(name) \
	{ \
		auto& bvec = base->name.inner(); \
		auto& cvec = changed->name.inner(); \
		auto sz = zc_max(bvec.capacity(), cvec.capacity()); \
		for (int i = 0; i < sz; ++i) \
			if(bvec[i] != cvec[i]) \
				tokens.push_back(fmt::format("{}[{}]={}", #name, i, (int)cvec[i])); \
	}
	LIST_ARRAY_PROPS;
	#undef BITSTR_PROP
	#undef VEC_PROP_2D
	#undef VEC_PROP
	#undef ARRAY_PROP
	
	return fmt::format("{}", fmt::join(tokens, " "));
}

// TODO use out param instead of allocating new zinitdata
zinitdata *apply_init_data_delta(zinitdata *base, string delta, string& out_error)
{
	zinitdata *result = new zinitdata(*base);
	if (delta.empty())
		return result;

	vector<string> tokens;
	util::split(delta, tokens, ' ');
	
	#define FAIL_IF(x, y) if (x) { \
		out_error = y; \
		delete result; \
		return nullptr; \
	}
	
	dword delta_version = 0;
	
	for (string token : tokens)
	{
		vector<string> kv;
		util::split(token, kv, '=');
		FAIL_IF(kv.size() != 2, fmt::format("invalid token '{}': expected one =", token));

		errno = 0;
		int as_int = std::strtol(kv[1].data(), nullptr, 10);
		FAIL_IF(errno, fmt::format("invalid token '{}': expected integer", token));
		
		if(kv[0] == "VERSION")
		{
			delta_version = as_int;
			continue;
		}
		bool is_array = kv[0].find('[') != string::npos;
		string key = kv[0];
		vector<int> index;
		if(is_array)
		{
			vector<string> name_index;
			util::split(kv[0], name_index, '[');

			errno = 0;
			for(int q = 1; q < name_index.size(); ++q)
				index.push_back(std::strtol(name_index[q].data(), nullptr, 10));
			
			FAIL_IF(errno, fmt::format("invalid token '{}': expected integer", token));
			key = name_index[0];
		}
		
		if(delta_version < DELTA_VERSION) //compat
		{
			#define DEPRPROP_IGNORE(name) if(key == #name) \
				continue;
			#define ARRAY_DEPRPROP_IOFS(old,new,indoffs) if (key == #old) \
			{ \
				FAIL_IF(index[0]+indoffs >= countof(result->new), fmt::format("invalid token '{}': integer too big", token)); \
				result->new[index[0]+indoffs] = as_int; \
				continue; \
			}
			#define DEPRPROP(old,new) if(key == #old) \
			{ \
				result->new = as_int; \
				continue; \
			}
			#define DEPRPROP_OFFS(old,new,op,offs) if(key == #old) \
			{ \
				result->new = as_int op offs; \
				continue; \
			}
			#define DEPRPROP_LITEM_BIT(old,flag) if(key == #old) \
			{ \
				for(int q = 0; q < 8; ++q) \
					SETFLAG(result->litems[index[0]+q], flag, get_bitl(as_int, index[0]+q)); \
				continue; \
			}
			if(delta_version < 1)
			{
				if(is_array)
				{
					if (key == "items")
					{
						FAIL_IF(index[0] >= 256, fmt::format("invalid token '{}': integer too big", token));
						result->set_item(index[0], as_int);
						continue;
					}
					if (key == "misc")
					{
						if(index[0] == 0)
							result->flags.set(INIT_FL_CONTPERCENT,as_int);
						else if(index[0] == 2)
							result->flags.set(INIT_FL_CANSLASH,as_int);
						continue;
					}
					if (key == "gen_doscript")
					{
						FAIL_IF(index[0] >= countof(result->items), fmt::format("invalid token '{}': integer too big", token));
						result->gen_doscript.set(index[0], as_int);
						continue;
					}
					ARRAY_DEPRPROP_IOFS(scrcnt,counter,crCUSTOM1);
					ARRAY_DEPRPROP_IOFS(scrmaxcnt,mcounter,crCUSTOM1);
				}
				else
				{
					DEPRPROP(super_bombs,counter[crSBOMBS]);
					DEPRPROP_OFFS(start_heart,counter[crLIFE],*,result->hp_per_heart);
					if (key == "triforce")
					{
						for(int q = 0; q < 8; ++q)
							SETFLAG(result->litems[q+1], (1 << li_mcguffin), get_bitl(as_int,q));
						continue;
					}
					DEPRPROP(rupies,counter[crMONEY]);
					DEPRPROP(keys,counter[crKEYS]);
					DEPRPROP(arrows,counter[crARROWS]);
					DEPRPROP(bombs,counter[crBOMBS]);
					DEPRPROP_OFFS(hc,counter[crLIFE],*,result->hp_per_heart);
					DEPRPROP(magic,counter[crMAGIC]);
					DEPRPROP(max_arrows,mcounter[crARROWS]);
					DEPRPROP(max_bombs,mcounter[crBOMBS]);
					DEPRPROP(max_keys,mcounter[crKEYS]);
					DEPRPROP(max_magic,mcounter[crMAGIC]);
					DEPRPROP(max_rupees,mcounter[crMONEY]);
					DEPRPROP(max_sbombs,mcounter[crSBOMBS]);
					DEPRPROP(gravity2,gravity);
					
					DEPRPROP_IGNORE(subscreen);
					DEPRPROP_IGNORE(subscreen_style);
					DEPRPROP_IGNORE(usecustomsfx);
				}
			}
			if(delta_version < 2)
			{
				DEPRPROP_LITEM_BIT(map, (1 << li_map))
				DEPRPROP_LITEM_BIT(compass, (1 << li_compass))
				DEPRPROP_LITEM_BIT(boss_key, (1 << li_boss_key))
				DEPRPROP_LITEM_BIT(mcguffin, (1 << li_mcguffin))
			}
			#undef DEPRPROP_LITEM_BIT
			#undef DEPRPROP_OFFS
			#undef DEPRPROP
			#undef ARRAY_DEPRPROP_IOFS
			#undef DEPRPROP_IGNORE
		}
		
		// Current
		if(is_array)
		{
			#define ARRAY_PROP(name) if (key == #name) \
			{ \
				FAIL_IF(index[0] >= countof(result->name), fmt::format("invalid token '{}': integer too big", token)); \
				result->name[index[0]] = as_int; \
				continue; \
			}
			#define VEC_PROP(name) if (key == #name) \
			{ \
				FAIL_IF(index[0] >= result->name.size(), fmt::format("invalid token '{}': integer too big", token)); \
				result->name[index[0]] = as_int; \
				continue; \
			}
			#define VEC_PROP_2D(name) if (key == #name) \
			{ \
				FAIL_IF(index.size() < 2, fmt::format("invalid token '{}': expected 2 indeces, got {}", token, index.size())); \
				FAIL_IF(index[0] >= result->name.size() \
					|| index[1] >= result->name[index[0]].size(), fmt::format("invalid token '{}': integer too big", token)); \
				result->name[index[0]][index[1]] = as_int; \
				continue; \
			}
			#define BITSTR_PROP(name) if (key == #name) \
			{ \
				FAIL_IF(index[0] >= 65536, fmt::format("invalid token '{}': integer too big", token)); \
				result->name.inner()[index[0]] = as_int; \
				continue; \
			}
			LIST_ARRAY_PROPS
			#undef BITSTR_PROP
			#undef VEC_PROP_2D
			#undef VEC_PROP
			#undef ARRAY_PROP
		}
		else
		{
			#define PROP(name) if (key == #name) \
			{ \
				result->name = as_int; \
				continue; \
			}
			#define ZFIXPROP(name) if (key == #name) \
			{ \
				result->name = zslongToFix(as_int); \
				continue; \
			}
			LIST_PROPS
			#undef ZFIXPROP
			#undef PROP
		}
		
		FAIL_IF(true, fmt::format("invalid token '{}': unknown property", token));
	}

	return result;
}

