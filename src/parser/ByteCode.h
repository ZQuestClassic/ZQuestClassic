
#ifndef BYTECODE_H //2.53 Updated to 16th Jan, 2017
#define BYTECODE_H

//#include "ScriptParser.h"
#include "ASTVisitors.h"
#include "DataStructs.h"
#include "Compiler.h"
#include "../zsyssimple.h"

#include <string>

using namespace std;

/*
 I will reserve the registers in the following scheme:
 SP - stack pointer
 D4 - stack frame pointer
 D6 - stack frame offset accumulator
 D2 - expression accumulator #1
 D3 - expression accumulator #2
 D0 - array index accumulator
 D1 - secondary array index accumulator
 D5 - pure SETR sink
 */
#define INDEX                   0
#define INDEX2                  1
#define EXP1                    2
#define EXP2                    3
#define SFRAME                  4
#define NUL                     5
#define SFTEMP                  6
#define WHAT_NO_7               7 // What, no 7?
#define WHAT_NO_8               8 // What, no 8?
#define ZELDAVERSION               9 // What, no 9?
#define SP                     10
#define DATA                   11
#define FCSET                  12
#define DELAY                  13
#define FX                     14
#define FY                     15
#define XD                     16
#define YD                     17
#define XD2                    18
#define YD2                    19
#define LINKX                  20
#define LINKY                  21
#define LINKDIR                22
#define LINKHP                 23
#define LINKMP                 24
#define LINKMAXHP              25
#define LINKMAXMP              26
#define LINKACTION             27
#define INPUTSTART             28
#define INPUTUP                29
#define INPUTDOWN              30
#define INPUTLEFT              31
#define INPUTRIGHT             32
#define INPUTA                 33
#define INPUTB                 34
#define INPUTL                 35
#define INPUTR                 36
#define SDD                    37 //  8 of these
#define COMBODD                38 // 176 of these
#define COMBOCD                39 // 176 of these
#define COMBOFD                40 // 176 of these
#define REFFFC                 41
#define REFITEM                42
#define ITEMCOUNT              43
#define ITEMX                  44
#define ITEMY                  45
#define ITEMDRAWTYPE           46
#define ITEMID                 47
#define ITEMTILE               48
#define ITEMCSET               49
#define ITEMFLASHCSET          50
#define ITEMFRAMES             51
#define ITEMFRAME              52
#define ITEMASPEED             53
#define ITEMDELAY              54
#define ITEMFLASH              55
#define ITEMFLIP               56
#define ITEMEXTEND             57
#define ITEMCLASSFAMILY        58
#define ITEMCLASSFAMTYPE       59
#define ITEMCLASSAMOUNT        60
#define ITEMCLASSMAX           61
#define ITEMCLASSSETMAX        62
#define ITEMCLASSSETGAME       63
#define ITEMCLASSCOUNTER       64
#define REFITEMCLASS           65
#define LINKZ                  66
#define LINKJUMP               67
#define ITEMZ                  68
#define ITEMJUMP               69
#define NPCZ                   70
#define NPCJUMP                71
#define WHAT_NO_72             72 // What, no 72?
#define LINKSWORDJINX          73
#define LINKITEMJINX           74
#define COMBOID                75
#define COMBOTD                76
#define COMBOSD                77
#define CURSCR                 78
#define CURMAP                 79
#define CURDMAP                80
#define GAMEDEATHS             81
#define GAMECHEAT              82
#define GAMETIME               83
#define GAMEHASPLAYED          84
#define GAMETIMEVALID          85
#define GAMEGUYCOUNT           86
#define GAMECONTSCR            87
#define GAMECONTDMAP           88
#define GAMECOUNTERD           89
#define GAMEMCOUNTERD          90
#define GAMEDCOUNTERD          91
#define GAMEGENERICD           92
#define GAMEITEMSD             93
#define GAMELITEMSD            94
#define GAMELKEYSD             95
#define SCREENSTATED           96
#define SCREENSTATEDD          97
#define SDDD                   98
#define FFFLAGSD               99
#define FFTWIDTH              100
#define FFTHEIGHT             101
#define FFCWIDTH              102
#define FFCHEIGHT             103
#define FFLINK                104
#define LINKITEMD             105
#define REFNPC                106
#define NPCCOUNT              107
#define NPCX                  108
#define NPCY                  109
#define NPCDIR                110
#define NPCRATE               111
#define NPCFRAMERATE          112
#define NPCHALTRATE           113
#define NPCDRAWTYPE           114
#define NPCHP                 115
#define NPCDP                 116
#define NPCWDP                117
#define NPCOTILE              118
#define NPCWEAPON             119
#define NPCITEMSET            120
#define NPCCSET               121
#define NPCBOSSPAL            122
#define NPCBGSFX              123
#define NPCEXTEND             124
#define SCRDOORD              125
#define CURDSCR               126
#define LINKHELD              127
#define NPCSTEP               128
#define ITEMOTILE             129
#define INPUTMOUSEX           130
#define INPUTMOUSEY           131
#define QUAKE                 132
#define WAVY                  133
#define NPCID                 134
#define ITEMCLASSUSESOUND     135
#define INPUTMOUSEZ           136
#define INPUTMOUSEB           137
#define REFLWPN               138
#define LWPNCOUNT             139
#define LWPNX                 140
#define LWPNY                 141
#define LWPNDIR               142
#define LWPNSTEP              143
#define LWPNANGULAR           144
#define LWPNANGLE             145
#define LWPNDRAWTYPE          146
#define LWPNPOWER             147
#define LWPNDEAD              148
#define LWPNID                149
#define LWPNTILE              150
#define LWPNCSET              151
#define LWPNFLASHCSET         152
#define LWPNFRAMES            153
#define LWPNFRAME             154
#define LWPNASPEED            155
#define LWPNFLASH             156
#define LWPNFLIP              157
#define LWPNEXTEND            158
#define LWPNOTILE             159
#define LWPNOCSET             160
#define LWPNZ                 161
#define LWPNJUMP              162
#define REFEWPN               163
#define EWPNX                 164
#define EWPNY                 165
#define EWPNDIR               166
#define EWPNSTEP              167
#define EWPNANGULAR           168
#define EWPNANGLE             169
#define EWPNDRAWTYPE          170
#define EWPNPOWER             171
#define EWPNDEAD              172
#define EWPNID                173
#define EWPNTILE              174
#define EWPNCSET              175
#define EWPNFLASHCSET         176
#define EWPNFRAMES            177
#define EWPNFRAME             178
#define EWPNASPEED            179
#define EWPNFLASH             180
#define EWPNFLIP              181
#define EWPNCOUNT             182
#define EWPNEXTEND            183
#define EWPNOTILE             184
#define EWPNOCSET             185
#define EWPNZ                 186
#define EWPNJUMP              187
#define COMBODDM              188 // 176 of these
#define COMBOCDM              189 // 176 of these
#define COMBOFDM              190 // 176 of these
#define COMBOIDM              191 // 176 of these
#define COMBOTDM              192 // 176 of these
#define COMBOSDM              193 // 176 of these
#define SCRIPTRAM			  194
#define GLOBALRAM			  195
#define SCRIPTRAMD			  196
#define GLOBALRAMD			  197
#define WHAT_NO_198		      198 //What, no 198?
#define LWPNHXOFS			  199
#define LWPNHYOFS			  200
#define LWPNXOFS			  201
#define LWPNYOFS			  202
#define LWPNZOFS			  203
#define LWPNHXSZ			  204
#define LWPNHYSZ			  205
#define LWPNHZSZ			  206
#define EWPNHXOFS			  207
#define EWPNHYOFS			  208
#define EWPNXOFS			  209
#define EWPNYOFS			  210
#define EWPNZOFS			  211
#define EWPNHXSZ			  212
#define EWPNHYSZ			  213
#define EWPNHZSZ			  214
#define NPCHXOFS			  215
#define NPCHYOFS			  216
#define NPCXOFS				  217
#define NPCYOFS				  218
#define NPCZOFS				  219
#define NPCHXSZ			      220
#define NPCHYSZ			      221
#define NPCHZSZ			      222
#define ITEMHXOFS			  223
#define ITEMHYOFS			  224
#define ITEMXOFS			  225
#define ITEMYOFS			  226
#define ITEMZOFS			  227
#define ITEMHXSZ			  228
#define ITEMHYSZ			  229
#define ITEMHZSZ			  230
#define LWPNTXSZ			  231
#define LWPNTYSZ			  232
#define EWPNTXSZ			  233
#define EWPNTYSZ			  234
#define NPCTXSZ				  235
#define NPCTYSZ				  236
#define ITEMTXSZ			  237
#define ITEMTYSZ			  238
#define LINKHXOFS			  239
#define LINKHYOFS			  240
#define LINKXOFS			  241
#define LINKYOFS			  242
#define LINKZOFS			  243
#define LINKHXSZ			  244
#define LINKHYSZ			  245
#define LINKHZSZ			  246
#define LINKTXSZ			  247
#define LINKTYSZ			  248
#define LINKDRUNK			  249
#define NPCTILE		 	 	  250
#define LWPNBEHIND            251
#define EWPNBEHIND            252
#define SDDDD                 253
#define CURLEVEL              254
#define ITEMPICKUP	      255
#define INPUTMAP              256
#define LIT                   257
#define INPUTEX1             258
#define INPUTEX2             259
#define INPUTEX3             260
#define INPUTEX4             261
#define INPUTPRESSSTART      262
#define INPUTPRESSUP         263
#define INPUTPRESSDOWN       264
#define INPUTPRESSLEFT       265
#define INPUTPRESSRIGHT      266
#define INPUTPRESSA          267
#define INPUTPRESSB          268
#define INPUTPRESSL          269
#define INPUTPRESSR          270
#define INPUTPRESSEX1        271
#define INPUTPRESSEX2        272
#define INPUTPRESSEX3        273
#define INPUTPRESSEX4        274
#define LWPNMISCD            275 //16 of these
#define EWPNMISCD            276 //16 of these
#define NPCMISCD             277 //16 of these
#define ITEMMISCD            278 //16 of these
#define FFMISCD              279 //16 of these
#define GETMIDI              280
#define NPCHOMING            281
#define NPCDD                282 //10 of these
#define LINKEQUIP            283

#define INPUTAXISUP			284
#define INPUTAXISDOWN		285
#define INPUTAXISLEFT		286
#define INPUTAXISRIGHT		287
#define INPUTPRESSAXISUP    288
#define INPUTPRESSAXISDOWN  289
#define INPUTPRESSAXISLEFT  290
#define INPUTPRESSAXISRIGHT 291

#define NPCTYPE			    292
#define FFSCRIPT			293
#define SCREENFLAGSD		294 //10 of these
#define LINKINVIS			295
#define LINKINVINC			296
#define SCREENEFLAGSD		297 //3 of these
#define NPCMFLAGS			298
#define FFINITDD			299 //8 of these
#define LINKMISCD           300 //16 of these
#define DMAPFLAGSD			301 //2 of these
#define LWPNCOLLDET			302
#define EWPNCOLLDET			303
#define NPCCOLLDET			304
#define LINKLADDERX		    305
#define LINKLADDERY		    306
#define NPCSTUN			    307
#define NPCDEFENSED		    308
#define ITEMCLASSPOWER      309
#define DMAPLEVELD			310 //512 of these
#define DMAPCOMPASSD		311 //512 of these
#define DMAPCONTINUED		312 //512 of these
#define DMAPMIDID			313 //512 of these
#define ITEMCLASSINITDD		314 //8 of these
#define LINKHITDIR			315
#define ROOMTYPE			316
#define ROOMDATA			317
#define LINKTILE			318
#define LINKFLIP			319
#define INPUTPRESSMAP		320
#define NPCHUNGER			321
#define GAMESTANDALONE		322
#define GAMEENTRSCR			323
#define GAMEENTRDMAP		324
#define GAMECLICKFREEZE		325
#define PUSHBLOCKX			326
#define PUSHBLOCKY			327
#define PUSHBLOCKCOMBO		328
#define PUSHBLOCKCSET		329
#define UNDERCOMBO			330
#define UNDERCSET			331
#define DMAPOFFSET			332 //512 of these
#define DMAPMAP				333 //512 of these
//#define FFDD			    309 //8 of these

#define __RESERVED_FOR_GAMETHROTTLE	334
#define LINKDIAG	335
#define LINKBIGHITBOX	336
#define ITEMCLASSID 337
#define _DEPRECATED_IDATAMISC1 338
#define _DEPRECATED_IDATAMISC2 339
#define _DEPRECATED_IDATAMISC3 340
#define _DEPRECATED_IDATAMISC4 341
#define _DEPRECATED_IDATAMISC5 342
#define _DEPRECATED_IDATAMISC6 343
#define _DEPRECATED_IDATAMISC7 344
#define _DEPRECATED_IDATAMISC8 345
#define _DEPRECATED_IDATAMISC9 346
#define _DEPRECATED_IDATAMISC10 347
#define _DEPRECATED_IDATAWPN1 348
#define _DEPRECATED_IDATAWPN2 349
#define _DEPRECATED_IDATAWPN3 350
#define _DEPRECATED_IDATAWPN4 351
#define _DEPRECATED_IDATAWPN5 352
#define _DEPRECATED_IDATAWPN6 353
#define _DEPRECATED_IDATAWPN7 354
#define _DEPRECATED_IDATAWPN8 355
#define _DEPRECATED_IDATAWPN9 356
#define _DEPRECATED_IDATAWPN10 357
#define IDATALTM 358
#define IDATAWPN 359
#define IDATAMISC 360
#define IDATASCRIPT 361
#define IDATAPSCRIPT 362
#define IDATAMAGCOST 363
#define IDATAMINHEARTS 364
#define IDATATILE 365
#define IDATAUNUSED 366
#define IDATACSET 367
#define IDATAFRAMES 368
#define IDATAASPEED 369
#define IDATADELAY 370
#define IDATACOMBINE 371
#define IDATADOWNGRADE 372
#define _DEPRECATED_IDATAFLAG1 373
#define _DEPRECATED_IDATAFLAG2 374
#define IDATAKEEPOLD 375
#define IDATARUPEECOST 376
#define IDATAEDIBLE 377
#define IDATAFLAGUNUSED 378
#define IDATAGAINLOWER 379
#define _DEPRECATED_IDATAFLAG3 380
#define _DEPRECATED_IDATAFLAG4 381
#define _DEPRECATED_IDATAFLAG5 382

#define __RESERVED_FOR_LINKRETSQUARE 383
#define LINKUSINGITEM 384
#define LINKUSINGITEMA 385
#define LINKUSINGITEMB 386
#define __RESERVED_FOR_LINKWARPSOUND 387
#define __RESERVED_FOR_PLAYWARPSOUND 388
#define __RESERVED_FOR_WARPEFFECT 389
#define __RESERVED_FOR_PLAYPITWARPSFX 390
#define LINKEATEN 391
#define __RESERVED_FOR_LINKEXTEND 392
#define __RESERVED_FOR_SETLINKEXTEND 393
#define __RESERVED_FOR_GETLINKEXTEND 394
#define NPCSCRDEFENSED 395
#define __RESERVED_FOR_SETLINKTILE 396
#define __RESERVED_FOR_SIDEWARPSFX 397
#define __RESERVED_FOR_PITWARPSFX 398
#define __RESERVED_FOR_SIDEWARPVISUAL 399
#define __RESERVED_FOR_PITWARPVISUAL 400
#define GAMESETA 401
#define GAMESETB 402
#define SETITEMSLOT 403
#define BUTTONPRESS 404
#define BUTTONINPUT 405
#define BUTTONHELD 406
#define KEYPRESS 407
#define READKEY 408
#define JOYPADPRESS 409
#define DISABLEDITEM 410
#define RESERVED0007 411
#define RESERVED0008 412
#define RESERVED0009 413
#define RESERVED000A 414
#define RESERVED000B 415
#define RESERVED000C 416
#define RESERVED000D 417
#define RESERVED000E 418
#define RESERVED000F 419
#define LINKITEMB 420 
#define LINKITEMA 421
#define __RESERVED_FOR_LINKWALKTILE 422
#define	__RESERVED_FOR_LINKFLOATTILE 423
#define __RESERVED_FOR_LINKSWIMTILE 424
#define	__RESERVED_FOR_LINKDIVETILE 425
#define	__RESERVED_FOR_LINKSLASHTILE 426
#define	__RESERVED_FOR_LINKJUMPTILE 427
#define	__RESERVED_FOR_LINKCHARGETILE 428
#define	__RESERVED_FOR_LINKSTABTILE 429
#define	__RESERVED_FOR_LINKCASTTILE 430
#define	__RESERVED_FOR_LINKHOLD1LTILE 431
#define	__RESERVED_FOR_LINKHOLD2LTILE 432
#define	__RESERVED_FOR_LINKHOLD1WTILE 433
#define	__RESERVED_FOR_LINKHOLD2WTILE 434
#define	__RESERVED_FOR_LINKPOUNDTILE 435
#define __RESERVED_FOR_LINKSWIMSPD 436
#define __RESERVED_FOR_LINKWALKANMSPD 437
#define __RESERVED_FOR_LINKANIMTYPE 438
#define LINKINVFRAME 439
#define LINKCANFLICKER 440
#define LINKHURTSFX 441
#define NOACTIVESUBSC 442
#define LWPNRANGE 443
#define UNUSED444 444
#define ZELDABUILD 445
#define ZELDABETA 446
#define NPCINVINC 447
#define NPCSUPERMAN 448
#define NPCHASITEM 449
#define NPCRINGLEAD 450
#define IDATAFRAME 451
#define ITEMACLK 452
#define FFCID 453
#define IDATAFLAGS 454
#define IDATASPRITE 455
#define IDATAATTRIB 456
#define DMAPLEVELPAL 457 //512 of these
#define __RESERVED_FOR_ITEMINDEX 458	
#define __RESERVED_FOR_LWPNINDEX 459
#define __RESERVED_FOR_EWPNINDEX 460
#define __RESERVED_FOR_NPCINDEX 461
#define __RESERVED_FOR_ITEMPTR 462
#define __RESERVED_FOR_NPCPTR 463
#define __RESERVED_FOR_LWPNPTR 464
#define __RESERVED_FOR_EWPNPTR 465

//array pointer typecasting
#define SETSCREENDOOR 466
#define SETSCREENENEMY 467
#define GAMEMAXMAPS 468
#define CREATELWPNDX 469
#define __RESERVED_FOR_SCREENFLAG 470
#define ADJUSTSFX 471
#define RESVD112E 472

//Reserved values for cooperative editing
#define RESVD112F 473
#define RESVD1130 474
#define RESVD1131 475
#define RESVD1132 476
#define RESVD1133 477
#define RESVD1134 478
#define RESVD1135 479
#define RESVD1136 480
#define RESVD1137 481
#define RESVD1138 482
#define RESVD1139 483
#define IDATACOSTCOUNTER 484
#define IDATAOVERRIDEFLWEAP 485
#define IDATATILEHWEAP 486
#define IDATATILEWWEAP 487
#define IDATAHZSZWEAP 488
#define IDATAHYSZWEAP 489
#define IDATAHXSZWEAP 490
#define IDATADYOFSWEAP 491
#define IDATADXOFSWEAP 492
#define IDATAHYOFSWEAP 493
#define IDATAHXOFSWEAP 494
#define IDATAOVERRIDEFL 495
#define IDATAPICKUP 496
#define IDATATILEH 497
#define IDATATILEW 498
#define IDATAHZSZ 499
#define IDATAHYSZ 500
#define IDATAHXSZ 501
#define IDATADYOFS 502
#define IDATADXOFS 503
#define IDATAHYOFS 504
#define IDATAHXOFS 505
#define IDATAUSEWPN 506
#define IDATAUSEDEF 507
#define IDATAWRANGE 508
#define IDATAUSEMVT 509
#define IDATADURATION 510

#define IDATADUPLICATES 511
#define IDATADRAWLAYER 512
#define IDATACOLLECTFLAGS 513
#define IDATAWEAPONSCRIPT 514
#define IDATAMISCD 515
#define IDATAWEAPHXOFS 516
#define IDATAWEAPHYOFS 517
#define IDATAWEAPHYSZ 518
#define IDATAWEAPHXSZ 519
#define IDATAWEAPHZSZ 520
#define IDATAWEAPXOFS 521
#define IDATAWEAPYOFS 522
#define IDATAWEAPZOFS 523
#define NPCWEAPSPRITE 524
#define IDATAWPNINITD 525

#define DEBUGREFFFC 526
#define DEBUGREFITEM 527
#define DEBUGREFNPC 528
#define DEBUGREFITEMDATA 529
#define DEBUGREFLWEAPON 530
#define DEBUGREFEWEAPON 531
#define DEBUGSP 532
#define DEBUGGDR 533

#define SCREENWIDTH 534
#define SCREENHEIGHT 535
#define SCREENVIEWX 536
#define SCREENVIEWY 537 
#define SCREENGUY 538
#define SCREENSTRING 539
#define SCREENROOM 540
#define SCREENENTX 541
#define SCREENENTY 542
#define SCREENITEM 543
#define SCREENUNDCMB 544
#define SCREENUNDCST 545
#define SCREENCATCH 546
#define SETSCREENLAYOP 547
#define SETSCREENSECCMB 548
#define SETSCREENSECCST 549
#define SETSCREENSECFLG 550
#define SETSCREENLAYMAP 551
#define SETSCREENLAYSCR 552
#define SETSCREENPATH 553
#define SETSCREENWARPRX 554
#define SETSCREENWARPRY 555
#define GAMENUMMESSAGES 556
#define GAMESUBSCHEIGHT 557
#define GAMEPLAYFIELDOFS 558
#define PASSSUBOFS 559
//NPCData
//three inputs, no return
#define SETNPCDATASCRIPTDEF 560
#define SETNPCDATADEFENSE 561
#define SETNPCDATASIZEFLAG 562
#define SETNPCDATAATTRIBUTE 563
#define SCDBLOCKWEAPON 564
#define SCDEXPANSION 565
#define SCDSTRIKEWEAPONS 566
#define SETGAMEOVERELEMENT 567
#define SETGAMEOVERSTRING 568
#define MOUSEARR 569
#define REFMAPDATA 570
#define REFSCREENDATA 571
#define REFCOMBODATA 572
#define REFSPRITEDATA 573
#define REFBITMAP 574
#define REFNPCCLASS 575
#define IDATAMAGICTIMER 576

//New DataTypes
//spritedata sd->
#define SPRITEDATATILE 577
#define SPRITEDATAMISC 578
#define SPRITEDATACSETS 579
#define SPRITEDATAFRAMES 580
#define SPRITEDATASPEED 581
#define SPRITEDATATYPE 582

//npcdata nd->
#define NPCDATATILE 583
#define NPCDATAHEIGHT 584
#define NPCDATAFLAGS 585
#define NPCDATAFLAGS2 586
#define NPCDATAWIDTH 587
#define NPCDATAHITSFX 588
#define NPCDATASTILE 589
#define NPCDATASWIDTH 590
#define NPCDATASHEIGHT 591
#define NPCDATAETILE 592
#define NPCDATAEWIDTH 593
#define NPCDATAEHEIGHT 594
#define NPCDATAHP 595
#define NPCDATAFAMILY 596
#define NPCDATACSET 597
#define NPCDATAANIM 598
#define NPCDATAEANIM 599
#define NPCDATAFRAMERATE 600
#define NPCDATAEFRAMERATE 601
#define NPCDATATOUCHDAMAGE 602
#define NPCDATAWEAPONDAMAGE 603
#define NPCDATAWEAPON 604
#define NPCDATARANDOM 605
#define NPCDATAHALT 606
#define NPCDATASTEP 607
#define NPCDATAHOMING 608
#define NPCDATAHUNGER 609
#define NPCDATADROPSET 610
#define NPCDATABGSFX 611
#define NPCDATADEATHSFX 612
#define NPCDATAXOFS 613
#define NPCDATAYOFS 614
#define NPCDATAZOFS 615
#define NPCDATAHXOFS 616
#define NPCDATAHYOFS 617
#define NPCDATAHITWIDTH 618
#define NPCDATAHITHEIGHT 619
#define NPCDATAHITZ 620
#define NPCDATATILEWIDTH 621
#define NPCDATATILEHEIGHT 622
#define NPCDATAWPNSPRITE 623
#define NPCDATADEFENSE 624
#define NPCDATASIZEFLAG 625
#define NPCDATAATTRIBUTE 626
#define NPCDATASHIELD 627
#define NPCDATAFROZENTILE 628
#define NPCDATAFROZENCSET 629
#define NPCDATARESERVED005 630
#define NPCDATARESERVED006 631
#define NPCDATARESERVED007 632
#define NPCDATARESERVED008 633
#define NPCDATARESERVED009 634
#define NPCDATARESERVED010 635
#define NPCDATARESERVED011 636
#define NPCDATARESERVED012 637
#define NPCDATARESERVED013 638
#define NPCDATARESERVED014 639
#define NPCDATARESERVED015 640
#define NPCDATARESERVED016 641
#define NPCDATARESERVED017 642
#define NPCDATARESERVED018 643
#define NPCDATARESERVED019 644

#define MAPDATAVALID		645	//b
#define MAPDATAGUY 		646	//b
#define MAPDATASTRING		647	//w
#define MAPDATAROOM 		648	//b
#define MAPDATAITEM 		649	//b
#define MAPDATAHASITEM 		650	//b
#define MAPDATATILEWARPTYPE 	651	//b, 4 of these
#define MAPDATATILEWARPOVFLAGS 	652	//b, tilewarpoverlayflags
#define MAPDATADOORCOMBOSET 	653	//w
#define MAPDATAWARPRETX 	654	//b, 4 of these
#define MAPDATAWARPRETY 	655	//b, 4 of these
#define MAPDATAWARPRETURNC 	656	//w
#define MAPDATASTAIRX 		657	//b
#define MAPDATASTAIRY 		658	//b
#define MAPDATACOLOUR 		659	//w
#define MAPDATAENEMYFLAGS 	660	//b
#define MAPDATADOOR 		661	//b, 4 of these
#define MAPDATATILEWARPDMAP 	662	//w, 4 of these
#define MAPDATATILEWARPSCREEN 	663	//b, 4 of these
#define MAPDATAEXITDIR 		664	//b
#define MAPDATAENEMY 		665	//w, 10 of these
#define MAPDATAPATTERN 		666	//b
#define MAPDATASIDEWARPTYPE 	667	//b, 4 of these
#define MAPDATASIDEWARPOVFLAGS 	668	//b
#define MAPDATAWARPARRIVALX 	669	//b
#define MAPDATAWARPARRIVALY 	670	//b
#define MAPDATAPATH 		671	//b, 4 of these
#define MAPDATASIDEWARPSC 	672	//b, 4 of these
#define MAPDATASIDEWARPDMAP 	673	//w, 4 of these
#define MAPDATASIDEWARPINDEX 	674	//b
#define MAPDATAUNDERCOMBO 	675	//w
#define MAPDATAUNDERCSET 	676	//b
#define MAPDATACATCHALL 	677	//W
#define MAPDATAFLAGS 		678	//B, 11 OF THESE, flags, flags2-flags10
#define MAPDATACSENSITIVE 	679	//B
#define MAPDATANORESET 		680	//W
#define MAPDATANOCARRY 		681	//W
#define MAPDATALAYERMAP 	682	//B, 6 OF THESE
#define MAPDATALAYERSCREEN 	683	//B, 6 OF THESE
#define MAPDATALAYEROPACITY 	684	//B, 6 OF THESE
#define MAPDATATIMEDWARPTICS 	685	//W
#define MAPDATANEXTMAP 		686	//B
#define MAPDATANEXTSCREEN 	687	//B
#define MAPDATASECRETCOMBO 	688	//W, 128 OF THESE
#define MAPDATASECRETCSET 	689	//B, 128 OF THESE
#define MAPDATASECRETFLAG 	690	//B, 128 OF THESE
#define MAPDATAVIEWX 		691	//W
#define MAPDATAVIEWY 		692	//W
#define MAPDATASCREENWIDTH 	693	//B
#define MAPDATASCREENHEIGHT 	694	//B
#define MAPDATAENTRYX 		695	//B
#define MAPDATAENTRYY 		696	//B
#define MAPDATANUMFF 		697	//INT16
#define MAPDATAFFDATA 		698	//W, 32 OF THESE
#define MAPDATAFFCSET 		699	//B, 32
#define MAPDATAFFDELAY 		700	//W, 32
#define MAPDATAFFX 		701	//INT32, 32 OF THESE
#define MAPDATAFFY 		702	//..
#define MAPDATAFFXDELTA 	703	//..
#define MAPDATAFFYDELTA 	704	//..
#define MAPDATAFFXDELTA2 	705	//..
#define MAPDATAFFYDELTA2 	706	//..
#define MAPDATAFFFLAGS 		707	//INT16, 23 OF THESE
#define MAPDATAFFWIDTH 		708	//B, 32 OF THESE
#define MAPDATAFFHEIGHT 	709	//B, 32 OF THESE
#define MAPDATAFFLINK 		710	//B, 32 OF THESE
#define MAPDATAFFSCRIPT 	711	//W, 32 OF THESE
#define MAPDATAINTID 		712	//INT32 , 32 OF THESE, EACH WITH 10 INDICES. 
#define MAPDATAINITA 		713	//INT32, 32 OF THESE, EACH WITH 2
#define MAPDATAFFINITIALISED 	714	//BOOL, 32 OF THESE
#define MAPDATASCRIPTENTRY 	715	//W
#define MAPDATASCRIPTOCCUPANCY 	716	//W
#define MAPDATASCRIPTEXIT 	717	//W
#define MAPDATAOCEANSFX 	718	//B
#define MAPDATABOSSSFX 		719	//B
#define MAPDATASECRETSFX 	720	//B
#define MAPDATAHOLDUPSFX 	721	//B
#define MAPDATASCREENMIDI 	722	//SHORT, OLD QUESTS ONLY?
#define MAPDATALENSLAYER 	723	//B, OLD QUESTS ONLY?

#define SCREENDATAVALID			724	//b
#define SCREENDATAGUY 			725	//b
#define SCREENDATASTRING		726	//w
#define SCREENDATAROOM 			727	//b
#define SCREENDATAITEM 			728	//b
#define SCREENDATAHASITEM 		729	//b
#define SCREENDATATILEWARPTYPE 		730	//b, 4 of these
#define SCREENDATATILEWARPOVFLAGS 	731	//b, tilewarpoverlayflags
#define SCREENDATADOORCOMBOSET 		732	//w
#define SCREENDATAWARPRETX 		733	//b, 4 of these
#define SCREENDATAWARPRETY 		734	//b, 4 of these
#define SCREENDATAWARPRETURNC 		735	//w
#define SCREENDATASTAIRX 		736	//b
#define SCREENDATASTAIRY 		737	//b
#define SCREENDATACOLOUR 		738	//w
#define SCREENDATAENEMYFLAGS 		739	//b
#define SCREENDATADOOR 			740	//b, 4 of these
#define SCREENDATATILEWARPDMAP 		741	//w, 4 of these
#define SCREENDATATILEWARPSCREEN 	742	//b, 4 of these
#define SCREENDATAEXITDIR 		743	//b
#define SCREENDATAENEMY 		744	//w, 10 of these
#define SCREENDATAPATTERN 		745	//b
#define SCREENDATASIDEWARPTYPE 		746	//b, 4 of these
#define SCREENDATASIDEWARPOVFLAGS 	747	//b
#define SCREENDATAWARPARRIVALX 		748	//b
#define SCREENDATAWARPARRIVALY 		749	//b
#define SCREENDATAPATH 			750	//b, 4 of these
#define SCREENDATASIDEWARPSC 		751	//b, 4 of these
#define SCREENDATASIDEWARPDMAP 		752	//w, 4 of these
#define SCREENDATASIDEWARPINDEX 	753	//b
#define SCREENDATAUNDERCOMBO 		754	//w
#define SCREENDATAUNDERCSET 		755	//b
#define SCREENDATACATCHALL 		756	//W
#define SCREENDATAFLAGS 		757	//B, 11 OF THESE, flags, flags2-flags10
#define SCREENDATACSENSITIVE 		758	//B
#define SCREENDATANORESET 		759	//W
#define SCREENDATANOCARRY 		760	//W
#define SCREENDATALAYERMAP 		761	//B, 6 OF THESE
#define SCREENDATALAYERSCREEN 		762	//B, 6 OF THESE
#define SCREENDATALAYEROPACITY 		763	//B, 6 OF THESE
#define SCREENDATATIMEDWARPTICS 	764	//W
#define SCREENDATANEXTMAP 		765	//B
#define SCREENDATANEXTSCREEN 		766	//B
#define SCREENDATASECRETCOMBO 		767	//W, 128 OF THESE
#define SCREENDATASECRETCSET 		768	//B, 128 OF THESE
#define SCREENDATASECRETFLAG 		769	//B, 128 OF THESE
#define SCREENDATAVIEWX 		770	//W
#define SCREENDATAVIEWY 		771	//W
#define SCREENDATASCREENWIDTH 		772	//B
#define SCREENDATASCREENHEIGHT 		773	//B
#define SCREENDATAENTRYX 		774	//B
#define SCREENDATAENTRYY 		775	//B
#define SCREENDATANUMFF 		776	//INT16
#define SCREENDATAFFDATA 		777	//W, 32 OF THESE
#define SCREENDATAFFCSET 		778	//B, 32
#define SCREENDATAFFDELAY 		779	//W, 32
#define SCREENDATAFFX 			780	//INT32, 32 OF THESE
#define SCREENDATAFFY 			781	//..
#define SCREENDATAFFXDELTA 		782	//..
#define SCREENDATAFFYDELTA 		783	//..
#define SCREENDATAFFXDELTA2 		784	//..
#define SCREENDATAFFYDELTA2 		785	//..
#define SCREENDATAFFFLAGS 		786	//INT16, 23 OF THESE
#define SCREENDATAFFWIDTH 		787	//B, 32 OF THESE
#define SCREENDATAFFHEIGHT 		788	//B, 32 OF THESE
#define SCREENDATAFFLINK 		789	//B, 32 OF THESE
#define SCREENDATAFFSCRIPT 		790	//W, 32 OF THESE
#define SCREENDATAINTID 		791	//INT32 , 32 OF THESE, EACH WITH 10 INDICES. 
#define SCREENDATAINITA 		792	//INT32, 32 OF THESE, EACH WITH 2
#define SCREENDATAFFINITIALISED 	793	//BOOL, 32 OF THESE
#define SCREENDATASCRIPTENTRY 		794	//W
#define SCREENDATASCRIPTOCCUPANCY 	795	//W
#define SCREENDATASCRIPTEXIT 		796	//W
#define SCREENDATAOCEANSFX 		797	//B
#define SCREENDATABOSSSFX 		798	//B
#define SCREENDATASECRETSFX 		799	//B
#define SCREENDATAHOLDUPSFX 		800	//B
#define SCREENDATASCREENMIDI 		801	//SHORT, OLD QUESTS ONLY?
#define SCREENDATALENSLAYER 		802	//B, OLD QUESTS ONLY?

#define LINKSCRIPTTILE 			803
#define MAPDATAITEMX 			804
#define MAPDATAITEMY 			805
#define SCREENDATAITEMX 		806
#define SCREENDATAITEMY 		807

#define MAPDATAFFEFFECTWIDTH 		808
#define MAPDATAFFEFFECTHEIGHT 		809
#define SCREENDATAFFEFFECTWIDTH 	810
#define SCREENDATAFFEFFECTHEIGHT 	811

#define MAPDATAMISCD 			812

#define MAPDATACOMBODD 			813
#define MAPDATACOMBOCD 			814
#define MAPDATACOMBOFD 			815
#define MAPDATACOMBOTD 			816
#define MAPDATACOMBOID 			817
#define MAPDATACOMBOSD 			818

#define MAPDATASCREENSTATED 		819
#define MAPDATASCREENFLAGSD 		820
#define MAPDATASCREENEFLAGSD 		821
#define LINKSCRIPFLIP	 		822
#define LOADMAPDATA 			823

#define REFDMAPDATA 			824
#define REFSHOPDATA 			825
#define REFMSGDATA 			826
#define REFNIL 				827
#define LWPNPARENT 			828
#define LWPNLEVEL 			829
#define EWPNPARENT 			830
#define SHOPDATANAME 			831
#define SHOPDATAITEM 			832
#define SHOPDATAHASITEM 		833
#define SHOPDATAPRICE 			834
#define SHOPDATASTRING 			835
#define NPCSHIELD 			836
#define AUDIOVOLUME 			837
#define AUDIOPAN 			838

#define MESSAGEDATANEXT 	839	//W
#define MESSAGEDATATILE 	840	//W
#define MESSAGEDATACSET 	841	//b
#define MESSAGEDATATRANS 	842	//BOOL
#define MESSAGEDATAFONT 	843	//B
#define MESSAGEDATAX 		844	//SHORT
#define MESSAGEDATAY 		845	//SHORT
#define MESSAGEDATAW 		846	//UNSIGNED SHORT
#define MESSAGEDATAH 		847	//UNSIGNED SHORT
#define MESSAGEDATASFX 		848	//BYTE
#define MESSAGEDATALISTPOS 	849	//WORD
#define MESSAGEDATAVSPACE 	850	//BYTE
#define MESSAGEDATAHSPACE 	851	//BYTE
#define MESSAGEDATAFLAGS 	852	//BYTE

#define DMAPDATAMAP 		383	//byte
#define DMAPDATALEVEL 		854	//word
#define DMAPDATAOFFSET 		855	//char
#define DMAPDATACOMPASS 	856	//byte
#define DMAPDATAPALETTE 	857	//word
#define DMAPDATAMIDI 		858	//byte
#define DMAPDATACONTINUE 	859	//byte
#define DMAPDATATYPE 		860	//byte
#define DMAPDATAGRID 		861	//byte[8] --array
#define DMAPDATAMINIMAPTILE 	862	//word - two of these, so let's do MinimapTile[2]
#define DMAPDATAMINIMAPCSET 	863	//byte - two of these, so let's do MinimapCSet[2]
#define DMAPDATALARGEMAPTILE 	864	//word -- two of these, so let's to LargemapTile[2]
#define DMAPDATALARGEMAPCSET 	865	//word -- two of these, so let's to LargemaCSet[2]
#define DMAPDATAMUISCTRACK 	866	//byte
#define DMAPDATASUBSCRA 	867	//byte, active subscreen
#define DMAPDATASUBSCRP 	868	//byte, passive subscreen
#define DMAPDATADISABLEDITEMS 	869	//byte[iMax]
#define DMAPDATAFLAGS 		870	//long
#define NPCFROZEN 		871	//long
#define NPCFROZENTILE 		872	//long
#define NPCFROZENCSET 		873	//long
#define IDATAPSTRING 		874	//word

#define ITEMPSTRING 		875	//word
#define ITEMPSTRINGFLAGS 	876	//word
#define ITEMOVERRIDEFLAGS 	877	//long

#define LINKPUSH 		878	
#define GAMEMISC 		879	//long
#define LINKSTUN 		880	//int (32b)
#define TYPINGMODE 		881	//bool, ffcore
//#define DMAPDATAGRAVITY 	871	//unimplemented
//#define DMAPDATAJUMPLAYER 	872	//unimplemented

//NEWCOMBO STRUCT

#define COMBODTILE		882	//word
#define COMBODFLIP		883	//char
#define COMBODWALK		884	//char
#define COMBODTYPE		885	//char
#define COMBODCSET		886	//C
#define COMBODFOO		887	//W
#define COMBODFRAMES		888	//C
#define COMBODNEXTD		889	//W
#define COMBODNEXTC		890	//C
#define COMBODFLAG		891	//C
#define COMBODSKIPANIM		893	//C
#define COMBODAKIMANIMY		894	//C
#define COMBODANIMFLAGS		895	//C
#define COMBODEXPANSION		896	//C , 6 INDICES
#define COMBODATTRIBUTES 	897	//LONG, 4 INDICES, INDIVIDUAL VALUES
#define COMBODUSRFLAGS		898	//LONG
#define COMBODTRIGGERFLAGS	899	//LONG 3 INDICES AS FLAGSETS
#define COMBODTRIGGERLEVEL	900	//LONG

//COMBOCLASS STRUCT
#define COMBODNAME		901	//CHAR[64], STRING
#define COMBODBLOCKNPC		902	//C
#define COMBODBLOCKHOLE		903	//C
#define COMBODBLOCKTRIG		904	//C
#define COMBODBLOCKWEAPON	905	//C, 32 INDICES
#define COMBODCONVXSPEED	906	//SHORT
#define COMBODCONVYSPEED	907	//SHORT
#define COMBODSPAWNNPC		908	//W
#define COMBODSPAWNNPCWHEN	909	//C
#define COMBODSPAWNNPCCHANGE	910	//LONG
#define COMBODDIRCHANGETYPE	911	//C
#define COMBODDISTANCECHANGETILES	912	//LONG
#define COMBODDIVEITEM		913	//SHORT
#define COMBODDOCK		914	//C
#define COMBODFAIRY		915	//C
#define COMBODFFATTRCHANGE	916	//C
#define COMBODFOORDECOTILE	917	//LONG
#define COMBODFOORDECOTYPE	918	//C
#define COMBODHOOKSHOTPOINT	919	//C
#define COMBODLADDERPASS	920	//C
#define COMBODLOCKBLOCK		921	//C
#define COMBODLOCKBLOCKCHANGE	922	//LONG
#define COMBODMAGICMIRROR	923	//C
#define COMBODMODHPAMOUNT	924	//SHORT
#define COMBODMODHPDELAY	925	//C
#define COMBODMODHPTYPE		926	//C
#define COMBODNMODMPAMOUNT	927	//SHORT
#define COMBODMODMPDELAY	928	//C
#define COMBODMODMPTYPE		929	//C
#define COMBODNOPUSHBLOCK	930	//C
#define COMBODOVERHEAD		931	//C
#define COMBODPLACENPC		932	//C
#define COMBODPUSHDIR		933	//C
#define COMBODPUSHWAIT		934	//C
#define COMBODPUSHHEAVY		935	//C
#define COMBODPUSHED		936	//C
#define COMBODRAFT		937	//C
#define COMBODRESETROOM		938	//C
#define COMBODSAVEPOINTTYPE	939	//C
#define COMBODSCREENFREEZETYPE	940	//C
#define COMBODSECRETCOMBO	941	//C
#define COMBODSINGULAR		942	//C
#define COMBODSLOWWALK		943	//C
#define COMBODSTATUETYPE	944	//C
#define COMBODSTEPTYPE		945	//C
#define COMBODSTEPCHANGEINTO	946	//LONG
#define COMBODSTRIKEWEAPONS	947	//BYTE, 32 INDICES. 
#define COMBODSTRIKEREMNANTS	948	//LONG
#define COMBODSTRIKEREMNANTSTYPE 949	//C
#define COMBODSTRIKECHANGE	950	//LONG
#define COMBODSTRIKEITEM	951	//SHORT
#define COMBODTOUCHITEM		952	//SHORT
#define COMBODTOUCHSTAIRS	953	//C
#define COMBODTRIGGERTYPE	954	//C
#define COMBODTRIGGERSENS	955	//C
#define COMBODWARPTYPE		956	//C
#define COMBODWARPSENS		957	//C
#define COMBODWARPDIRECT	958	//C
#define COMBODWARPLOCATION	959	//C
#define COMBODWATER		960	//C
#define COMBODWHISTLE		961	//C
#define COMBODWINGAME		962	//C
#define COMBODBLOCKWPNLEVEL	963	//C
#define COMBODNEXTTIMER		964	//W

#define REFDROPS		965
#define REFPONDS 		966
#define REFWARPRINGS 		967
#define REFDOORS 		968
#define REFUICOLOURS 		969
#define REFRGB 			970
#define REFPALETTE 		971
#define REFTUNES 		972
#define REFPALCYCLE 		973
#define REFGAMEDATA 		974
#define REFCHEATS 		975
#define LINKHITBY 		976
#define LINKDEFENCE 		977
#define NPCHITBY 		978
#define NPCISCORE 		979
#define NPCSCRIPTUID 		980
#define LWEAPONSCRIPTUID 	981
#define EWEAPONSCRIPTUID 	982
#define ITEMSCRIPTUID 		983
#define DMAPDATASIDEVIEW 	984

#define DONULL 	985
#define DEBUGD 	986
#define GETPIXEL 	987
#define DOUNTYPE 	988
#define IDATAKEEP 	989

#define LAST_BYTECODE 		990

//END OF BYTECODE

namespace ZScript
{
	class LiteralArgument;
	class VarArgument;
	class LabelArgument;
	class GlobalArgument;

	class ArgumentVisitor
	{
	public:
		virtual void caseLiteral(LiteralArgument &host, void *param)
		{
			void *temp;
			temp=&host;
			param=param; /*these are here to bypass compiler warnings about unused arguments*/
		}
		virtual void caseVar(VarArgument &host, void *param)
		{
			void *temp;
			temp=&host;
			param=param; /*these are here to bypass compiler warnings about unused arguments*/
		}
		virtual void caseLabel(LabelArgument &host, void *param)
		{
			void *temp;
			temp=&host;
			param=param; /*these are here to bypass compiler warnings about unused arguments*/
		}
		virtual void caseGlobal(GlobalArgument &host, void *param)
		{
			void *temp;
			temp=&host;
			param=param; /*these are here to bypass compiler warnings about unused arguments*/
		}
		virtual ~ArgumentVisitor() {}
	};

	class Argument
	{
	public:
		virtual string toString()=0;
		virtual void execute(ArgumentVisitor &host, void *param)=0;
		virtual Argument *clone()=0;
		virtual ~Argument() {}
	};

	class LiteralArgument : public Argument
	{
	public:
		LiteralArgument(long Value) : value(Value) {}
		string toString();
		void execute(ArgumentVisitor &host, void *param)
		{
			host.caseLiteral(*this, param);
		}
		Argument *clone()
		{
			return new LiteralArgument(value);
		}
	private:
		long value;
	};

	class VarArgument : public Argument
	{
	public:
		VarArgument(int id) : ID(id) {}
		string toString();
		void execute(ArgumentVisitor &host, void *param)
		{
			host.caseVar(*this,param);
		}
		Argument *clone()
		{
			return new VarArgument(ID);
		}
	private:
		int ID;
	};

	class GlobalArgument : public Argument
	{
	public:
		GlobalArgument(int id) : ID(id) {}
		string toString();
		void execute(ArgumentVisitor &host, void *param)
		{
			host.caseGlobal(*this,param);
		}
		Argument *clone()
		{
			return new GlobalArgument(ID);
		}
	private:
		int ID;
	};

	class LabelArgument : public Argument
	{
	public:
		LabelArgument(int id) : ID(id), haslineno(false) {}
		string toString();
		void execute(ArgumentVisitor &host, void *param)
		{
			host.caseLabel(*this,param);
		}
		Argument *clone()
		{
			return new LabelArgument(ID);
		}
		int getID()
		{
			return ID;
		}
		void setLineNo(int l)
		{
			haslineno=true;
			lineno=l;
		}
	private:
		int ID;
		int lineno;
		bool haslineno;
	};

	class UnaryOpcode : public Opcode
	{
	public:
		UnaryOpcode(Argument *A) : a(A) {}
		~UnaryOpcode()
		{
			delete a;
		}
		Argument *getArgument()
		{
			return a;
		}
		void execute(ArgumentVisitor &host, void *param)
		{
			a->execute(host, param);
		}
	protected:
		Argument *a;
	};

	class BinaryOpcode : public Opcode
	{
	public:
		BinaryOpcode(Argument *A, Argument *B) : a(A), b(B) {}
		~BinaryOpcode()
		{
			delete a;
			delete b;
		}
		Argument *getFirstArgument()
		{
			return a;
		}
		Argument *getSecondArgument()
		{
			return b;
		}
		void execute(ArgumentVisitor &host, void *param)
		{
			a->execute(host, param);
			b->execute(host, param);
		}
	protected:
		Argument *a;
		Argument *b;
	};

	class OSetTrue : public UnaryOpcode
	{
	public:
		OSetTrue(Argument *A) : UnaryOpcode(A) {}
		string toString();
		Opcode *clone()
		{
			return new OSetTrue(a->clone());
		}
	};

	class OSetFalse : public UnaryOpcode
	{
	public:
		OSetFalse(Argument *A) : UnaryOpcode(A) {}
		string toString();
		Opcode *clone()
		{
			return new OSetFalse(a->clone());
		}
	};

	class OSetMore : public UnaryOpcode
	{
	public:
		OSetMore(Argument *A) : UnaryOpcode(A) {}
		string toString();
		Opcode *clone()
		{
			return new OSetMore(a->clone());
		}
	};

	class OSetLess : public UnaryOpcode
	{
	public:
		OSetLess(Argument *A) : UnaryOpcode(A) {}
		string toString();
		Opcode *clone()
		{
			return new OSetLess(a->clone());
		}
	};

	class OSetImmediate : public BinaryOpcode
	{
	public:
		OSetImmediate(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OSetImmediate(a->clone(),b->clone());
		}
	};

	class OSetRegister : public BinaryOpcode
	{
	public:
		OSetRegister(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OSetRegister(a->clone(),b->clone());
		}
	};

	class OAddImmediate : public BinaryOpcode
	{
	public:
		OAddImmediate(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OAddImmediate(a->clone(),b->clone());
		}
	};

	class OAddRegister : public BinaryOpcode
	{
	public:
		OAddRegister(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OAddRegister(a->clone(),b->clone());
		}
	};

	class OSubImmediate : public BinaryOpcode
	{
	public:
		OSubImmediate(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OSubImmediate(a->clone(),b->clone());
		}
	};

	class OSubRegister : public BinaryOpcode
	{
	public:
		OSubRegister(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OSubRegister(a->clone(),b->clone());
		}
	};

	class OMultImmediate : public BinaryOpcode
	{
	public:
		OMultImmediate(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OMultImmediate(a->clone(),b->clone());
		}
	};

	class OMultRegister : public BinaryOpcode
	{
	public:
		OMultRegister(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OMultRegister(a->clone(),b->clone());
		}
	};

	class ODivImmediate : public BinaryOpcode
	{
	public:
		ODivImmediate(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new ODivImmediate(a->clone(),b->clone());
		}
	};

	class ODivRegister : public BinaryOpcode
	{
	public:
		ODivRegister(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new ODivRegister(a->clone(),b->clone());
		}
	};

	class OCompareImmediate : public BinaryOpcode
	{
	public:
		OCompareImmediate(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OCompareImmediate(a->clone(),b->clone());
		}
	};

	class OCompareRegister : public BinaryOpcode
	{
	public:
		OCompareRegister(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OCompareRegister(a->clone(),b->clone());
		}
	};

	class OWaitframe : public Opcode
	{
	public:
		string toString();
		Opcode *clone()
		{
			return new OWaitframe();
		}
	};

	class OWaitdraw : public Opcode
	{
	public:
		string toString();
		Opcode *clone()
		{
			return new OWaitdraw();
		}
	};

	class OGotoImmediate : public UnaryOpcode
	{
	public:
		OGotoImmediate(Argument *A) : UnaryOpcode(A) {}
		string toString();
		Opcode *clone()
		{
			return new OGotoImmediate(a->clone());
		}
	};

	class OGotoTrueImmediate: public UnaryOpcode
	{
	public:
		OGotoTrueImmediate(Argument *A) : UnaryOpcode(A) {}
		string toString();
		Opcode *clone()
		{
			return new OGotoTrueImmediate(a->clone());
		}
	};

	class OGotoFalseImmediate: public UnaryOpcode
	{
	public:
		OGotoFalseImmediate(Argument *A) : UnaryOpcode(A) {}
		string toString();
		Opcode *clone()
		{
			return new OGotoFalseImmediate(a->clone());
		}
	};

	class OGotoMoreImmediate : public UnaryOpcode
	{
	public:
		OGotoMoreImmediate(Argument *A) : UnaryOpcode(A) {}
		string toString();
		Opcode *clone()
		{
			return new OGotoMoreImmediate(a->clone());
		}
	};

	class OGotoLessImmediate : public UnaryOpcode
	{
	public:
		OGotoLessImmediate(Argument *A) : UnaryOpcode(A) {}
		string toString();
		Opcode *clone()
		{
			return new OGotoLessImmediate(a->clone());
		}
	};

	class OPushRegister : public UnaryOpcode
	{
	public:
		OPushRegister(Argument *A) : UnaryOpcode(A) {}
		string toString();
		Opcode *clone()
		{
			return new OPushRegister(a->clone());
		}
	};

	class OPopRegister : public UnaryOpcode
	{
	public:
		OPopRegister(Argument *A) : UnaryOpcode(A) {}
		string toString();
		Opcode *clone()
		{
			return new OPopRegister(a->clone());
		}
	};

	class OLoadIndirect : public BinaryOpcode
	{
	public:
		OLoadIndirect(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OLoadIndirect(a->clone(),b->clone());
		}
	};

	class OStoreIndirect : public BinaryOpcode
	{
	public:
		OStoreIndirect(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OStoreIndirect(a->clone(),b->clone());
		}
	};

	class OQuit : public Opcode
	{
	public:
		string toString();
		Opcode *clone()
		{
			return new OQuit();
		}
	};

	class OGotoRegister : public UnaryOpcode
	{
	public:
		OGotoRegister(Argument *A) : UnaryOpcode(A) {}
		string toString();
		Opcode *clone()
		{
			return new OGotoRegister(a->clone());
		}
	};

	class OTraceRegister : public UnaryOpcode
	{
	public:
		OTraceRegister(Argument *A) : UnaryOpcode(A) {}
		string toString();
		Opcode *clone()
		{
			return new OTraceRegister(a->clone());
		}
	};

	class OTrace2Register : public UnaryOpcode
	{
	public:
		OTrace2Register(Argument *A) : UnaryOpcode(A) {}
		string toString();
		Opcode *clone()
		{
			return new OTrace2Register(a->clone());
		}
	};

	class OTrace3 : public Opcode
	{
	public:
		string toString();
		Opcode *clone()
		{
			return new OTrace3();
		}
	};

	class OTrace4 : public Opcode
	{
	public:
		string toString();
		Opcode *clone()
		{
			return new OTrace4();
		}
	};

	class OTrace5Register : public Opcode
	{
	public:
		string toString();
		Opcode *clone()
		{
			return new OTrace5Register();
		}
	};

	class OTrace6Register : public UnaryOpcode
	{
	public:
		OTrace6Register(Argument *A) : UnaryOpcode(A) {}
		string toString();
		Opcode *clone()
		{
			return new OTrace6Register(a->clone());
		}
	};

	class OAndImmediate : public BinaryOpcode
	{
	public:
		OAndImmediate(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OAndImmediate(a->clone(),b->clone());
		}
	};

	class OAndRegister : public BinaryOpcode
	{
	public:
		OAndRegister(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OAndRegister(a->clone(),b->clone());
		}
	};

	class OOrImmediate : public BinaryOpcode
	{
	public:
		OOrImmediate(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OOrImmediate(a->clone(),b->clone());
		}
	};

	class OOrRegister : public BinaryOpcode
	{
	public:
		OOrRegister(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OOrRegister(a->clone(),b->clone());
		}
	};

	class OXorImmediate : public BinaryOpcode
	{
	public:
		OXorImmediate(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OXorImmediate(a->clone(), b->clone());
		}
	};

	class OXorRegister : public BinaryOpcode
	{
	public:
		OXorRegister(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OXorRegister(a->clone(), b->clone());
		}
	};

	class ONot : public UnaryOpcode
	{
	public:
		ONot(Argument *A) : UnaryOpcode(A) {}
		string toString();
		Opcode *clone()
		{
			return new ONot(a->clone());
		}
	};

	class OLShiftImmediate : public BinaryOpcode
	{
	public:
		OLShiftImmediate(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OLShiftImmediate(a->clone(), b->clone());
		}
	};

	class OLShiftRegister : public BinaryOpcode
	{
	public:
		OLShiftRegister(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OLShiftRegister(a->clone(), b->clone());
		}
	};

	class ORShiftImmediate : public BinaryOpcode
	{
	public:
		ORShiftImmediate(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new ORShiftImmediate(a->clone(), b->clone());
		}
	};

	class ORShiftRegister : public BinaryOpcode
	{
	public:
		ORShiftRegister(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new ORShiftRegister(a->clone(), b->clone());
		}
	};

	class OModuloImmediate : public BinaryOpcode
	{
	public:
		OModuloImmediate(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OModuloImmediate(a->clone(), b->clone());
		}
	};

	class OModuloRegister : public BinaryOpcode
	{
	public:
		OModuloRegister(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OModuloRegister(a->clone(), b->clone());
		}
	};

	class OSinRegister : public BinaryOpcode
	{
	public:
		OSinRegister(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OSinRegister(a->clone(), b->clone());
		}
	};

	class OArcSinRegister : public BinaryOpcode
	{
	public:
		OArcSinRegister(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OArcSinRegister(a->clone(), b->clone());
		}
	};

	class OCosRegister : public BinaryOpcode
	{
	public:
		OCosRegister(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OCosRegister(a->clone(), b->clone());
		}
	};

	class OArcCosRegister : public BinaryOpcode
	{
	public:
		OArcCosRegister(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OArcCosRegister(a->clone(), b->clone());
		}
	};

	class OTanRegister : public BinaryOpcode
	{
	public:
		OTanRegister(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OTanRegister(a->clone(), b->clone());
		}
	};

	class OATanRegister : public UnaryOpcode
	{
	public:
		OATanRegister(Argument *A) : UnaryOpcode(A) {}
		string toString();
		Opcode *clone()
		{
			return new OATanRegister(a->clone());
		}
	};

	class OMaxRegister : public BinaryOpcode
	{
	public:
		OMaxRegister(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OMaxRegister(a->clone(), b->clone());
		}
	};

	class OMinRegister : public BinaryOpcode
	{
	public:
		OMinRegister(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OMinRegister(a->clone(), b->clone());
		}
	};

	class OPowRegister : public BinaryOpcode
	{
	public:
		OPowRegister(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OPowRegister(a->clone(), b->clone());
		}
	};

	class OInvPowRegister : public BinaryOpcode
	{
	public:
		OInvPowRegister(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OInvPowRegister(a->clone(), b->clone());
		}
	};

	class OFactorial : public UnaryOpcode
	{
	public:
		OFactorial(Argument *A) : UnaryOpcode(A) {}
		string toString();
		Opcode *clone()
		{
			return new OFactorial(a->clone());
		}
	};

	class OAbsRegister : public UnaryOpcode
	{
	public:
		OAbsRegister(Argument *A) : UnaryOpcode(A) {}
		string toString();
		Opcode *clone()
		{
			return new OAbsRegister(a->clone());
		}
	};

	class OLog10Register : public UnaryOpcode
	{
	public:
		OLog10Register(Argument *A) : UnaryOpcode(A) {}
		string toString();
		Opcode *clone()
		{
			return new OLog10Register(a->clone());
		}
	};

	class OLogERegister : public UnaryOpcode
	{
	public:
		OLogERegister(Argument *A) : UnaryOpcode(A) {}
		string toString();
		Opcode *clone()
		{
			return new OLogERegister(a->clone());
		}
	};

	class OArraySize : public UnaryOpcode
	{
	public:
		OArraySize(Argument *A) : UnaryOpcode(A) {}
		string toString();
		Opcode *clone()
		{
			return new OArraySize(a->clone());
		}
	};


	class OArraySizeF : public UnaryOpcode
	{
	public:
		OArraySizeF(Argument *A) : UnaryOpcode(A) {}
		string toString();
		Opcode *clone()
		{
			return new OArraySizeF(a->clone());
		}
	};
	class OArraySizeN : public UnaryOpcode
	{
	public:
		OArraySizeN(Argument *A) : UnaryOpcode(A) {}
		string toString();
		Opcode *clone()
		{
			return new OArraySizeN(a->clone());
		}
	};
	class OArraySizeE : public UnaryOpcode
	{
	public:
		OArraySizeE(Argument *A) : UnaryOpcode(A) {}
		string toString();
		Opcode *clone()
		{
			return new OArraySizeE(a->clone());
		}
	};
	class OArraySizeL : public UnaryOpcode
	{
	public:
		OArraySizeL(Argument *A) : UnaryOpcode(A) {}
		string toString();
		Opcode *clone()
		{
			return new OArraySizeL(a->clone());
		}
	};
	class OArraySizeB : public UnaryOpcode
	{
	public:
		OArraySizeB(Argument *A) : UnaryOpcode(A) {}
		string toString();
		Opcode *clone()
		{
			return new OArraySizeB(a->clone());
		}
	};
	class OArraySizeI : public UnaryOpcode
	{
	public:
		OArraySizeI(Argument *A) : UnaryOpcode(A) {}
		string toString();
		Opcode *clone()
		{
			return new OArraySizeI(a->clone());
		}
	};
	class OArraySizeID : public UnaryOpcode
	{
	public:
		OArraySizeID(Argument *A) : UnaryOpcode(A) {}
		string toString();
		Opcode *clone()
		{
			return new OArraySizeID(a->clone());
		}
	};

	class OCheckTrig : public Opcode
	{
	public:
		string toString();
		Opcode *clone()
		{
			return new OCheckTrig();
		}
	};

	class ORandRegister : public BinaryOpcode
	{
	public:
		ORandRegister(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new ORandRegister(a->clone(), b->clone());
		}
	};

	class OSqrtRegister : public BinaryOpcode
	{
	public:
		OSqrtRegister(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OSqrtRegister(a->clone(),b->clone());
		}
	};

	class OCalcSplineRegister : public BinaryOpcode
	{
	public:
		OCalcSplineRegister(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OCalcSplineRegister(a->clone(),b->clone());
		}
	};

	class OSetColorRegister : public Opcode
	{
	public:
		string toString();
		Opcode *clone()
		{
			return new OSetColorRegister();
		}
	};

	class OSetDepthRegister : public Opcode
	{
	public:
		string toString();
		Opcode *clone()
		{
			return new OSetDepthRegister();
		}
	};

	class OCollisionRectRegister : public UnaryOpcode
	{
	public:
		OCollisionRectRegister(Argument *A) : UnaryOpcode(A) {}
		string toString();
		Opcode *clone()
		{
			return new OCollisionRectRegister(a->clone());
		}
	};

	class OCollisionBoxRegister : public UnaryOpcode
	{
	public:
		OCollisionBoxRegister(Argument *A) : UnaryOpcode(A) {}
		string toString();
		Opcode *clone()
		{
			return new OCollisionBoxRegister(a->clone());
		}
	};

	class OWarp : public BinaryOpcode
	{
	public:
		OWarp(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OWarp(a->clone(), b->clone());
		}
	};

	class OPitWarp : public BinaryOpcode
	{
	public:
		OPitWarp(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OPitWarp(a->clone(), b->clone());
		}
	};

	class OCreateItemRegister : public UnaryOpcode
	{
	public:
		OCreateItemRegister(Argument *A) : UnaryOpcode(A) {}
		string toString();
		Opcode *clone()
		{
			return new OCreateItemRegister(a->clone());
		}
	};

	class OCreateNPCRegister : public UnaryOpcode
	{
	public:
		OCreateNPCRegister(Argument *A) : UnaryOpcode(A) {}
		string toString();
		Opcode *clone()
		{
			return new OCreateNPCRegister(a->clone());
		}
	};

	class OCreateLWpnRegister : public UnaryOpcode
	{
	public:
		OCreateLWpnRegister(Argument *A) : UnaryOpcode(A) {}
		string toString();
		Opcode *clone()
		{
			return new OCreateLWpnRegister(a->clone());
		}
	};

	class OCreateEWpnRegister : public UnaryOpcode
	{
	public:
		OCreateEWpnRegister(Argument *A) : UnaryOpcode(A) {}
		string toString();
		Opcode *clone()
		{
			return new OCreateEWpnRegister(a->clone());
		}
	};

	class OLoadItemRegister : public UnaryOpcode
	{
	public:
		OLoadItemRegister(Argument *A) : UnaryOpcode(A) {}
		string toString();
		Opcode *clone()
		{
			return new OLoadItemRegister(a->clone());
		}
	};

	class OLoadItemDataRegister : public UnaryOpcode
	{
	public:
		OLoadItemDataRegister(Argument *A) : UnaryOpcode(A) {}
		string toString();
		Opcode *clone()
		{
			return new OLoadItemDataRegister(a->clone());
		}
	};

	class OLoadShopDataRegister : public UnaryOpcode
	{
	public:
		OLoadShopDataRegister(Argument *A) : UnaryOpcode(A) {}
		string toString();
		Opcode *clone()
		{
			return new OLoadShopDataRegister(a->clone());
		}
	};


	class OLoadInfoShopDataRegister : public UnaryOpcode
	{
	public:
		OLoadInfoShopDataRegister(Argument *A) : UnaryOpcode(A) {}
		string toString();
		Opcode *clone()
		{
			return new OLoadInfoShopDataRegister(a->clone());
		}
	};

	class OLoadNPCDataRegister : public UnaryOpcode
	{
	public:
		OLoadNPCDataRegister(Argument *A) : UnaryOpcode(A) {}
		string toString();
		Opcode *clone()
		{
			return new OLoadNPCDataRegister(a->clone());
		}
	};


	class OLoadMessageDataRegister : public UnaryOpcode
	{
	public:
		OLoadMessageDataRegister(Argument *A) : UnaryOpcode(A) {}
		string toString();
		Opcode *clone()
		{
			return new OLoadMessageDataRegister(a->clone());
		}
	};


	class OLoadDMapDataRegister : public UnaryOpcode
	{
	public:
		OLoadDMapDataRegister(Argument *A) : UnaryOpcode(A) {}
		string toString();
		Opcode *clone()
		{
			return new OLoadDMapDataRegister(a->clone());
		}
	};

	class ODMapDataGetNameRegister : public UnaryOpcode
	{
	public:
		ODMapDataGetNameRegister(Argument *A) : UnaryOpcode(A) {}
		string toString();
		Opcode *clone()
		{
			return new ODMapDataGetNameRegister(a->clone());
		}
	};

	class ODMapDataSetNameRegister : public UnaryOpcode
	{
	public:
		ODMapDataSetNameRegister(Argument *A) : UnaryOpcode(A) {}
		string toString();
		Opcode *clone()
		{
			return new ODMapDataSetNameRegister(a->clone());
		}
	};

	class ODMapDataGetTitleRegister : public UnaryOpcode
	{
	public:
		ODMapDataGetTitleRegister(Argument *A) : UnaryOpcode(A) {}
		string toString();
		Opcode *clone()
		{
			return new ODMapDataGetTitleRegister(a->clone());
		}
	};

	class ODMapDataSetTitleRegister : public UnaryOpcode
	{
	public:
		ODMapDataSetTitleRegister(Argument *A) : UnaryOpcode(A) {}
		string toString();
		Opcode *clone()
		{
			return new ODMapDataSetTitleRegister(a->clone());
		}
	};

	class ODMapDataGetIntroRegister : public UnaryOpcode
	{
	public:
		ODMapDataGetIntroRegister(Argument *A) : UnaryOpcode(A) {}
		string toString();
		Opcode *clone()
		{
			return new ODMapDataGetIntroRegister(a->clone());
		}
	};

	class ODMapDataSetIntroRegister : public UnaryOpcode
	{
	public:
		ODMapDataSetIntroRegister(Argument *A) : UnaryOpcode(A) {}
		string toString();
		Opcode *clone()
		{
			return new ODMapDataSetIntroRegister(a->clone());
		}
	};

	class ODMapDataGetMusicRegister : public UnaryOpcode
	{
	public:
		ODMapDataGetMusicRegister(Argument *A) : UnaryOpcode(A) {}
		string toString();
		Opcode *clone()
		{
			return new ODMapDataGetMusicRegister(a->clone());
		}
	};

	class ODMapDataSetMusicRegister : public UnaryOpcode
	{
	public:
		ODMapDataSetMusicRegister(Argument *A) : UnaryOpcode(A) {}
		string toString();
		Opcode *clone()
		{
			return new ODMapDataSetMusicRegister(a->clone());
		}
	};


	class OMessageDataSetStringRegister : public UnaryOpcode
	{
	public:
		OMessageDataSetStringRegister(Argument *A) : UnaryOpcode(A) {}
		string toString();
		Opcode *clone()
		{
			return new OMessageDataSetStringRegister(a->clone());
		}
	};


	class OMessageDataGetStringRegister : public UnaryOpcode
	{
	public:
		OMessageDataGetStringRegister(Argument *A) : UnaryOpcode(A) {}
		string toString();
		Opcode *clone()
		{
			return new OMessageDataGetStringRegister(a->clone());
		}
	};

	class OLoadComboDataRegister : public UnaryOpcode
	{
	public:
		OLoadComboDataRegister(Argument *A) : UnaryOpcode(A) {}
		string toString();
		Opcode *clone()
		{
			return new OLoadComboDataRegister(a->clone());
		}
	};

	class OLoadMapDataRegister : public BinaryOpcode
	{
	public:
		OLoadMapDataRegister(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OLoadMapDataRegister(a->clone(), b->clone());
		}
	};


	class OLoadSpriteDataRegister : public UnaryOpcode
	{
	public:
		OLoadSpriteDataRegister(Argument *A) : UnaryOpcode(A) {}
		string toString();
		Opcode *clone()
		{
			return new OLoadSpriteDataRegister(a->clone());
		}
	};


	class OLoadScreenDataRegister : public UnaryOpcode
	{
	public:
		OLoadScreenDataRegister(Argument *A) : UnaryOpcode(A) {}
		string toString();
		Opcode *clone()
		{
			return new OLoadScreenDataRegister(a->clone());
		}
	};


	class OLoadBitmapDataRegister : public UnaryOpcode
	{
	public:
		OLoadBitmapDataRegister(Argument *A) : UnaryOpcode(A) {}
		string toString();
		Opcode *clone()
		{
			return new OLoadBitmapDataRegister(a->clone());
		}
	};

	class OLoadNPCRegister : public UnaryOpcode
	{
	public:
		OLoadNPCRegister(Argument *A) : UnaryOpcode(A) {}
		string toString();
		Opcode *clone()
		{
			return new OLoadNPCRegister(a->clone());
		}
	};

	class OLoadLWpnRegister : public UnaryOpcode
	{
	public:
		OLoadLWpnRegister(Argument *A) : UnaryOpcode(A) {}
		string toString();
		Opcode *clone()
		{
			return new OLoadLWpnRegister(a->clone());
		}
	};

	class OLoadEWpnRegister : public UnaryOpcode
	{
	public:
		OLoadEWpnRegister(Argument *A) : UnaryOpcode(A) {}
		string toString();
		Opcode *clone()
		{
			return new OLoadEWpnRegister(a->clone());
		}
	};

	class OPlaySoundRegister : public UnaryOpcode
	{
	public:
		OPlaySoundRegister(Argument *A) : UnaryOpcode(A) {}
		string toString();
		Opcode *clone()
		{
			return new OPlaySoundRegister(a->clone());
		}
	};

	// Audio->


	class OAdjustVolumeRegister : public UnaryOpcode
	{
	public:
		OAdjustVolumeRegister(Argument *A) : UnaryOpcode(A) {}
		string toString();
		Opcode *clone()
		{
			return new OAdjustVolumeRegister(a->clone());
		}
	};


	class OAdjustSFXVolumeRegister : public UnaryOpcode
	{
	public:
		OAdjustSFXVolumeRegister(Argument *A) : UnaryOpcode(A) {}
		string toString();
		Opcode *clone()
		{
			return new OAdjustSFXVolumeRegister(a->clone());
		}
	};


	class OEndSoundRegister : public UnaryOpcode
	{
	public:
		OEndSoundRegister(Argument *A) : UnaryOpcode(A) {}
		string toString();
		Opcode *clone()
		{
			return new OEndSoundRegister(a->clone());
		}
	};


	class OPauseSoundRegister : public UnaryOpcode
	{
	public:
		OPauseSoundRegister(Argument *A) : UnaryOpcode(A) {}
		string toString();
		Opcode *clone()
		{
			return new OPauseSoundRegister(a->clone());
		}
	};


	class OResumeSoundRegister : public UnaryOpcode
	{
	public:
		OResumeSoundRegister(Argument *A) : UnaryOpcode(A) {}
		string toString();
		Opcode *clone()
		{
			return new OResumeSoundRegister(a->clone());
		}
	};


	class OPauseSFX : public UnaryOpcode
	{
	public:
		OPauseSFX(Argument *A) : UnaryOpcode(A) {}
		string toString();
		Opcode *clone()
		{
			return new OPauseSFX(a->clone());
		}
	};

	class OResumeSFX : public UnaryOpcode
	{
	public:
		OResumeSFX(Argument *A) : UnaryOpcode(A) {}
		string toString();
		Opcode *clone()
		{
			return new OResumeSFX(a->clone());
		}
	};

	class OContinueSFX : public UnaryOpcode
	{
	public:
		OContinueSFX(Argument *A) : UnaryOpcode(A) {}
		string toString();
		Opcode *clone()
		{
			return new OContinueSFX(a->clone());
		}
	};


	class OPauseMusic : public Opcode
	{
	public:
		string toString();
		Opcode *clone()
		{
			return new OPauseMusic();
		}
	};

	class OResumeMusic : public Opcode
	{
	public:
		string toString();
		Opcode *clone()
		{
			return new OResumeMusic();
		}
	};


	//END Audio

	class OPlayMIDIRegister : public UnaryOpcode
	{
	public:
		OPlayMIDIRegister(Argument *A) : UnaryOpcode(A) {}
		string toString();
		Opcode *clone()
		{
			return new OPlayMIDIRegister(a->clone());
		}
	};

	class OPlayEnhancedMusic : public BinaryOpcode
	{
	public:
		OPlayEnhancedMusic(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OPlayEnhancedMusic(a->clone(), b->clone());
		}
	};

	class OGetDMapMusicFilename : public BinaryOpcode
	{
	public:
		OGetDMapMusicFilename(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OGetDMapMusicFilename(a->clone(), b->clone());
		}
	};

	class OGetDMapMusicTrack : public UnaryOpcode
	{
	public:
		OGetDMapMusicTrack(Argument *A) : UnaryOpcode(A) {}
		string toString();
		Opcode *clone()
		{
			return new OGetDMapMusicTrack(a->clone());
		}
	};

	class OSetDMapEnhancedMusic : public Opcode
	{
	public:
		string toString();
		Opcode *clone()
		{
			return new OSetDMapEnhancedMusic();
		}
	};

	class OGetSaveName : public UnaryOpcode
	{
	public:
		OGetSaveName(Argument *A) : UnaryOpcode(A) {}
		string toString();
		Opcode *clone()
		{
			return new OGetSaveName(a->clone());
		}
	};

	class OGetDMapName : public BinaryOpcode
	{
	public:
		OGetDMapName(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OGetDMapName(a->clone(), b->clone());
		}
	};

	class OSetDMapName : public BinaryOpcode
	{
	public:
		OSetDMapName(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OSetDMapName(a->clone(), b->clone());
		}
	};

	class OSetDMapIntro : public BinaryOpcode
	{
	public:
		OSetDMapIntro(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OSetDMapIntro(a->clone(), b->clone());
		}
	};

	class OSetDMapTitle : public BinaryOpcode
	{
	public:
		OSetDMapTitle(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OSetDMapTitle(a->clone(), b->clone());
		}
	};


	class OSetMessage : public BinaryOpcode
	{
	public:
		OSetMessage(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OSetMessage(a->clone(), b->clone());
		}
	};


	class OGetDMapIntro : public BinaryOpcode
	{
	public:
		OGetDMapIntro(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OGetDMapIntro(a->clone(), b->clone());
		}
	};

	class OGetDMapTitle : public BinaryOpcode
	{
	public:
		OGetDMapTitle(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OGetDMapTitle(a->clone(), b->clone());
		}
	};

	class OSetSaveName : public UnaryOpcode
	{
	public:
		OSetSaveName(Argument *A) : UnaryOpcode(A) {}
		string toString();
		Opcode *clone()
		{
			return new OSetSaveName(a->clone());
		}
	};

	class OGetItemName : public UnaryOpcode
	{
	public:
		OGetItemName(Argument *A) : UnaryOpcode(A) {}
		string toString();
		Opcode *clone()
		{
			return new OGetItemName(a->clone());
		}
	};

	class OGetNPCName : public UnaryOpcode
	{
	public:
		OGetNPCName(Argument *A) : UnaryOpcode(A) {}
		string toString();
		Opcode *clone()
		{
			return new OGetNPCName(a->clone());
		}
	};

	class OGetMessage : public BinaryOpcode
	{
	public:
		OGetMessage(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OGetMessage(a->clone(), b->clone());
		}
	};

	class OClearSpritesRegister : public UnaryOpcode
	{
	public:
		OClearSpritesRegister(Argument *A) : UnaryOpcode(A) {}
		string toString();
		Opcode *clone()
		{
			return new OClearSpritesRegister(a->clone());
		}
	};

	class OMessageRegister : public UnaryOpcode
	{
	public:
		OMessageRegister(Argument *A) : UnaryOpcode(A) {}
		string toString();
		Opcode *clone()
		{
			return new OMessageRegister(a->clone());
		}
	};

	class OIsSolid : public UnaryOpcode
	{
	public:
		OIsSolid(Argument *A) : UnaryOpcode(A) {}
		string toString();
		Opcode *clone()
		{
			return new OIsSolid(a->clone());
		}
	};

	class OSetSideWarpRegister : public Opcode
	{
	public:
		string toString();
		Opcode *clone()
		{
			return new OSetSideWarpRegister();
		}
	};

	class OSetTileWarpRegister : public Opcode
	{
	public:
		string toString();
		Opcode *clone()
		{
			return new OSetTileWarpRegister();
		}
	};

	class OGetSideWarpDMap : public UnaryOpcode
	{
	public:
		OGetSideWarpDMap(Argument *A) : UnaryOpcode(A) {}
		string toString();
		Opcode *clone()
		{
			return new OGetSideWarpDMap(a->clone());
		}
	};

	class OGetSideWarpScreen : public UnaryOpcode
	{
	public:
		OGetSideWarpScreen(Argument *A) : UnaryOpcode(A) {}
		string toString();
		Opcode *clone()
		{
			return new OGetSideWarpScreen(a->clone());
		}
	};

	class OGetSideWarpType : public UnaryOpcode
	{
	public:
		OGetSideWarpType(Argument *A) : UnaryOpcode(A) {}
		string toString();
		Opcode *clone()
		{
			return new OGetSideWarpType(a->clone());
		}
	};

	class OGetTileWarpDMap : public UnaryOpcode
	{
	public:
		OGetTileWarpDMap(Argument *A) : UnaryOpcode(A) {}
		string toString();
		Opcode *clone()
		{
			return new OGetTileWarpDMap(a->clone());
		}
	};

	class OGetTileWarpScreen : public UnaryOpcode
	{
	public:
		OGetTileWarpScreen(Argument *A) : UnaryOpcode(A) {}
		string toString();
		Opcode *clone()
		{
			return new OGetTileWarpScreen(a->clone());
		}
	};

	class OGetTileWarpType : public UnaryOpcode
	{
	public:
		OGetTileWarpType(Argument *A) : UnaryOpcode(A) {}
		string toString();
		Opcode *clone()
		{
			return new OGetTileWarpType(a->clone());
		}
	};

	class OLayerScreenRegister : public BinaryOpcode
	{
	public:
		OLayerScreenRegister(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OLayerScreenRegister(a->clone(), b->clone());
		}
	};

	class OLayerMapRegister : public BinaryOpcode
	{
	public:
		OLayerMapRegister(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OLayerMapRegister(a->clone(), b->clone());
		}
	};

	class OTriggerSecrets : public Opcode
	{
	public:
		string toString();
		Opcode *clone()
		{
			return new OTriggerSecrets();
		}
	};

	class OIsValidItem : public UnaryOpcode
	{
	public:
		OIsValidItem(Argument *A) : UnaryOpcode(A) {}
		string toString();
		Opcode *clone()
		{
			return new OIsValidItem(a->clone());
		}
	};

	class OIsValidNPC : public UnaryOpcode
	{
	public:
		OIsValidNPC(Argument *A) : UnaryOpcode(A) {}
		string toString();
		Opcode *clone()
		{
			return new OIsValidNPC(a->clone());
		}
	};

	class OIsValidLWpn : public UnaryOpcode
	{
	public:
		OIsValidLWpn(Argument *A) : UnaryOpcode(A) {}
		string toString();
		Opcode *clone()
		{
			return new OIsValidLWpn(a->clone());
		}
	};

	class OIsValidEWpn : public UnaryOpcode
	{
	public:
		OIsValidEWpn(Argument *A) : UnaryOpcode(A) {}
		string toString();
		Opcode *clone()
		{
			return new OIsValidEWpn(a->clone());
		}
	};

	class OUseSpriteLWpn : public UnaryOpcode
	{
	public:
		OUseSpriteLWpn(Argument *A) : UnaryOpcode(A) {}
		string toString();
		Opcode *clone()
		{
			return new OUseSpriteLWpn(a->clone());
		}
	};

	class OUseSpriteEWpn : public UnaryOpcode
	{
	public:
		OUseSpriteEWpn(Argument *A) : UnaryOpcode(A) {}
		string toString();
		Opcode *clone()
		{
			return new OUseSpriteEWpn(a->clone());
		}
	};

	class ORectangleRegister : public Opcode
	{
	public:
		string toString();
		Opcode *clone()
		{
			return new ORectangleRegister();
		}
	};

	class OCircleRegister : public Opcode
	{
	public:
		string toString();
		Opcode *clone()
		{
			return new OCircleRegister();
		}
	};

	class OArcRegister : public Opcode
	{
	public:
		string toString();
		Opcode *clone()
		{
			return new OArcRegister();
		}
	};

	class OEllipseRegister : public Opcode
	{
	public:
		string toString();
		Opcode *clone()
		{
			return new OEllipseRegister();
		}
	};

	class OLineRegister : public Opcode
	{
	public:
		string toString();
		Opcode *clone()
		{
			return new OLineRegister();
		}
	};

	class OSplineRegister : public Opcode
	{
	public:
		string toString();
		Opcode *clone()
		{
			return new OSplineRegister();
		}
	};

	class OPutPixelRegister : public Opcode
	{
	public:
		string toString();
		Opcode *clone()
		{
			return new OPutPixelRegister();
		}
	};

	class ODrawCharRegister : public Opcode
	{
	public:
		string toString();
		Opcode *clone()
		{
			return new ODrawCharRegister();
		}
	};

	class ODrawIntRegister : public Opcode
	{
	public:
		string toString();
		Opcode *clone()
		{
			return new ODrawIntRegister();
		}
	};

	class ODrawTileRegister : public Opcode
	{
	public:
		string toString();
		Opcode *clone()
		{
			return new ODrawTileRegister();
		}
	};

	class ODrawComboRegister : public Opcode
	{
	public:
		string toString();
		Opcode *clone()
		{
			return new ODrawComboRegister();
		}
	};

	class OQuadRegister : public Opcode
	{
	public:
		string toString();
		Opcode *clone()
		{
			return new OQuadRegister();
		}
	};

	class OTriangleRegister : public Opcode
	{
	public:
		string toString();
		Opcode *clone()
		{
			return new OTriangleRegister();
		}
	};

	class OQuad3DRegister : public Opcode
	{
	public:
		string toString();
		Opcode *clone()
		{
			return new OQuad3DRegister();
		}
	};

	class OTriangle3DRegister : public Opcode
	{
	public:
		string toString();
		Opcode *clone()
		{
			return new OTriangle3DRegister();
		}
	};

	class OFastTileRegister : public Opcode
	{
	public:
		string toString();
		Opcode *clone()
		{
			return new OFastTileRegister();
		}
	};

	class OFastComboRegister : public Opcode
	{
	public:
		string toString();
		Opcode *clone()
		{
			return new OFastComboRegister();
		}
	};

	class ODrawStringRegister : public Opcode
	{
	public:
		string toString();
		Opcode *clone()
		{
			return new ODrawStringRegister();
		}
	};

	class ODrawLayerRegister : public Opcode
	{
	public:
		string toString();
		Opcode *clone()
		{
			return new ODrawLayerRegister();
		}
	};

	class ODrawScreenRegister : public Opcode
	{
	public:
		string toString();
		Opcode *clone()
		{
			return new ODrawScreenRegister();
		}
	};

	class ODrawBitmapRegister : public Opcode
	{
	public:
		string toString();
		Opcode *clone()
		{
			return new ODrawBitmapRegister();
		}
	};


	class ODrawBitmapExRegister : public Opcode
	{
	public:
		string toString();
		Opcode *clone()
		{
			return new ODrawBitmapExRegister();
		}
	};

	class OSetRenderTargetRegister : public Opcode
	{
	public:
		string toString();
		Opcode *clone()
		{
			return new OSetRenderTargetRegister();
		}
	};

	class OSetDepthBufferRegister : public Opcode
	{
	public:
		string toString();
		Opcode *clone()
		{
			return new OSetDepthBufferRegister();
		}
	};

	class OGetDepthBufferRegister : public Opcode
	{
	public:
		string toString();
		Opcode *clone()
		{
			return new OGetDepthBufferRegister();
		}
	};

	class OSetColorBufferRegister : public Opcode
	{
	public:
		string toString();
		Opcode *clone()
		{
			return new OSetColorBufferRegister();
		}
	};

	class OGetColorBufferRegister : public Opcode
	{
	public:
		string toString();
		Opcode *clone()
		{
			return new OGetColorBufferRegister();
		}
	};

	class OCopyTileRegister : public BinaryOpcode
	{
	public:
		OCopyTileRegister(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OCopyTileRegister(a->clone(),b->clone());
		}
	};


	class OOverlayTileRegister : public BinaryOpcode
	{
	public:
		OOverlayTileRegister(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OOverlayTileRegister(a->clone(),b->clone());
		}
	};

	class OSwapTileRegister : public BinaryOpcode
	{
	public:
		OSwapTileRegister(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OSwapTileRegister(a->clone(),b->clone());
		}
	};

	class OClearTileRegister : public UnaryOpcode
	{
	public:
		OClearTileRegister(Argument *A) : UnaryOpcode(A) {}
		string toString();
		Opcode *clone()
		{
			return new OClearTileRegister(a->clone());
		}
	};

	class OAllocateMemRegister : public BinaryOpcode
	{
	public:
		OAllocateMemRegister(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OAllocateMemRegister(a->clone(),b->clone());
		}
	};

	class OAllocateMemImmediate : public BinaryOpcode
	{
	public:
		OAllocateMemImmediate(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OAllocateMemImmediate(a->clone(),b->clone());
		}
	};

	class OAllocateGlobalMemImmediate : public BinaryOpcode
	{
	public:
		OAllocateGlobalMemImmediate(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OAllocateGlobalMemImmediate(a->clone(),b->clone());
		}
	};

	class OAllocateGlobalMemRegister : public BinaryOpcode
	{
	public:
		OAllocateGlobalMemRegister(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OAllocateGlobalMemRegister(a->clone(),b->clone());
		}
	};

	class ODeallocateMemRegister : public UnaryOpcode
	{
	public:
		ODeallocateMemRegister(Argument *A) : UnaryOpcode(A) {}
		string toString();
		Opcode *clone()
		{
			return new ODeallocateMemRegister(a->clone());
		}
	};

	class ODeallocateMemImmediate : public UnaryOpcode
	{
	public:
		ODeallocateMemImmediate(Argument *A) : UnaryOpcode(A) {}
		string toString();
		Opcode *clone()
		{
			return new ODeallocateMemImmediate(a->clone());
		}
	};

	class OSave : public Opcode
	{
	public:
		string toString();
		Opcode *clone()
		{
			return new OSave();
		}
	};

	class OGetScreenFlags : public UnaryOpcode
	{
	public:
		OGetScreenFlags(Argument *A) : UnaryOpcode(A) {}
		string toString();
		Opcode *clone()
		{
			return new OGetScreenFlags(a->clone());
		}
	};

	class OGetScreenEFlags : public UnaryOpcode
	{
	public:
		OGetScreenEFlags(Argument *A) : UnaryOpcode(A) {}
		string toString();
		Opcode *clone()
		{
			return new OGetScreenEFlags(a->clone());
		}
	};

	class OEnd : public Opcode
	{
	public:
		string toString();
		Opcode *clone()
		{
			return new OEnd();
		}
	};

	class OComboTile : public BinaryOpcode
	{
	public:
		OComboTile(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OComboTile(a->clone(), b->clone());
		}
	};

	class OBreakShield : public UnaryOpcode
	{
	public:
		OBreakShield(Argument *A) : UnaryOpcode(A) {}
		string toString();
		Opcode *clone()
		{
			return new OBreakShield(a->clone());
		}
	};

	class OShowSaveScreen : public UnaryOpcode
	{
	public:
		OShowSaveScreen(Argument *A) : UnaryOpcode(A) {}
		string toString();
		Opcode *clone()
		{
			return new OShowSaveScreen(a->clone());
		}
	};

	class OShowSaveQuitScreen : public Opcode
	{
	public:
		string toString();
		Opcode *clone()
		{
			return new OShowSaveQuitScreen();
		}
	};

	class OSelectAWeaponRegister : public UnaryOpcode
	{
	public:
		OSelectAWeaponRegister(Argument *A) : UnaryOpcode(A) {}
		string toString();
		Opcode *clone()
		{
			return new OSelectAWeaponRegister(a->clone());
		}
	};

	class OSelectBWeaponRegister : public UnaryOpcode
	{
	public:
		OSelectBWeaponRegister(Argument *A) : UnaryOpcode(A) {}
		string toString();
		Opcode *clone()
		{
			return new OSelectBWeaponRegister(a->clone());
		}
	};

	class OGetFFCScript : public UnaryOpcode
	{
	public:
		OGetFFCScript(Argument *A) : UnaryOpcode(A) {}
		string toString();
		Opcode *clone()
		{
			return new OGetFFCScript(a->clone());
		}
	};

	class OGetItemScript : public UnaryOpcode
	{
	public:
		OGetItemScript(Argument *A) : UnaryOpcode(A) {}
		string toString();
		Opcode *clone()
		{
			return new OGetItemScript(a->clone());
		}
	};



	//2,54

	class OZapIn : public Opcode
	{
	public:
		string toString();
		Opcode *clone()
		{
			return new OZapIn();
		}
	};

	class OZapOut : public Opcode
	{
	public:
		string toString();
		Opcode *clone()
		{
			return new OZapOut();
		}
	};


	class OGreyscaleOn : public Opcode
	{
	public:
		string toString();
		Opcode *clone()
		{
			return new OGreyscaleOn();
		}
	};

	class OGreyscaleOff : public Opcode
	{
	public:
		string toString();
		Opcode *clone()
		{
			return new OGreyscaleOff();
		}
	};


	//These need to be unary opcodes that accept bool linkvisible. 
	class OWavyIn : public Opcode
	{
	public:
		string toString();
		Opcode *clone()
		{
			return new OWavyIn();
		}
	};


	class OWavyOut : public Opcode
	{
	public:
		string toString();
		Opcode *clone()
		{
			return new OWavyOut();
		}
	};

	class OOpenWipe : public Opcode
	{
	public:
		string toString();
		Opcode *clone()
		{
			return new OOpenWipe();
		}
	};


	class OGetFFCPointer : public UnaryOpcode
	{
	public:
		OGetFFCPointer(Argument *A) : UnaryOpcode(A) {}
		string toString();
		Opcode *clone()
		{
			return new OGetFFCPointer(a->clone());
		}
	};

	class OSetFFCPointer : public UnaryOpcode
	{
	public:
		OSetFFCPointer(Argument *A) : UnaryOpcode(A) {}
		string toString();
		Opcode *clone()
		{
			return new OSetFFCPointer(a->clone());
		}
	};


	class OGetNPCPointer : public UnaryOpcode
	{
	public:
		OGetNPCPointer(Argument *A) : UnaryOpcode(A) {}
		string toString();
		Opcode *clone()
		{
			return new OGetNPCPointer(a->clone());
		}
	};


	class OSetNPCPointer : public UnaryOpcode
	{
	public:
		OSetNPCPointer(Argument *A) : UnaryOpcode(A) {}
		string toString();
		Opcode *clone()
		{
			return new OSetNPCPointer(a->clone());
		}
	};


	class OGetLWeaponPointer : public UnaryOpcode
	{
	public:
		OGetLWeaponPointer(Argument *A) : UnaryOpcode(A) {}
		string toString();
		Opcode *clone()
		{
			return new OGetLWeaponPointer(a->clone());
		}
	};


	class OSetLWeaponPointer : public UnaryOpcode
	{
	public:
		OSetLWeaponPointer(Argument *A) : UnaryOpcode(A) {}
		string toString();
		Opcode *clone()
		{
			return new OSetLWeaponPointer(a->clone());
		}
	};


	class OGetEWeaponPointer : public UnaryOpcode
	{
	public:
		OGetEWeaponPointer(Argument *A) : UnaryOpcode(A) {}
		string toString();
		Opcode *clone()
		{
			return new OGetEWeaponPointer(a->clone());
		}
	};


	class OSetEWeaponPointer : public UnaryOpcode
	{
	public:
		OSetEWeaponPointer(Argument *A) : UnaryOpcode(A) {}
		string toString();
		Opcode *clone()
		{
			return new OSetEWeaponPointer(a->clone());
		}
	};


	class OGetItemPointer : public UnaryOpcode
	{
	public:
		OGetItemPointer(Argument *A) : UnaryOpcode(A) {}
		string toString();
		Opcode *clone()
		{
			return new OGetItemPointer(a->clone());
		}
	};


	class OSetItemPointer : public UnaryOpcode
	{
	public:
		OSetItemPointer(Argument *A) : UnaryOpcode(A) {}
		string toString();
		Opcode *clone()
		{
			return new OSetItemPointer(a->clone());
		}
	};


	class OGetItemDataPointer : public UnaryOpcode
	{
	public:
		OGetItemDataPointer(Argument *A) : UnaryOpcode(A) {}
		string toString();
		Opcode *clone()
		{
			return new OGetItemDataPointer(a->clone());
		}
	};


	class OSetItemDataPointer : public UnaryOpcode
	{
	public:
		OSetItemDataPointer(Argument *A) : UnaryOpcode(A) {}
		string toString();
		Opcode *clone()
		{
			return new OSetItemDataPointer(a->clone());
		}
	};


	class OGetBoolPointer : public UnaryOpcode
	{
	public:
		OGetBoolPointer(Argument *A) : UnaryOpcode(A) {}
		string toString();
		Opcode *clone()
		{
			return new OGetBoolPointer(a->clone());
		}
	};


	class OSetBoolPointer : public UnaryOpcode
	{
	public:
		OSetBoolPointer(Argument *A) : UnaryOpcode(A) {}
		string toString();
		Opcode *clone()
		{
			return new OSetBoolPointer(a->clone());
		}
	};


	class OGetScreenDoor : public UnaryOpcode
	{
	public:
		OGetScreenDoor(Argument *A) : UnaryOpcode(A) {}
		string toString();
		Opcode *clone()
		{
			return new OGetScreenDoor(a->clone());
		}
	};


	class OGetScreenEnemy : public UnaryOpcode
	{
	public:
		OGetScreenEnemy(Argument *A) : UnaryOpcode(A) {}
		string toString();
		Opcode *clone()
		{
			return new OGetScreenEnemy(a->clone());
		}
	};

	class OGetScreenLayerOpacity : public UnaryOpcode
	{
	public:
		OGetScreenLayerOpacity(Argument *A) : UnaryOpcode(A) {}
		string toString();
		Opcode *clone()
		{
			return new OGetScreenLayerOpacity(a->clone());
		}
	};
	class OGetScreenSecretCombo : public UnaryOpcode
	{
	public:
		OGetScreenSecretCombo(Argument *A) : UnaryOpcode(A) {}
		string toString();
		Opcode *clone()
		{
			return new OGetScreenSecretCombo(a->clone());
		}
	};
	class OGetScreenSecretCSet : public UnaryOpcode
	{
	public:
		OGetScreenSecretCSet(Argument *A) : UnaryOpcode(A) {}
		string toString();
		Opcode *clone()
		{
			return new OGetScreenSecretCSet(a->clone());
		}
	};
	class OGetScreenSecretFlag : public UnaryOpcode
	{
	public:
		OGetScreenSecretFlag(Argument *A) : UnaryOpcode(A) {}
		string toString();
		Opcode *clone()
		{
			return new OGetScreenSecretFlag(a->clone());
		}
	};
	class OGetScreenLayerMap : public UnaryOpcode
	{
	public:
		OGetScreenLayerMap(Argument *A) : UnaryOpcode(A) {}
		string toString();
		Opcode *clone()
		{
			return new OGetScreenLayerMap(a->clone());
		}
	};
	class OGetScreenLayerScreen : public UnaryOpcode
	{
	public:
		OGetScreenLayerScreen(Argument *A) : UnaryOpcode(A) {}
		string toString();
		Opcode *clone()
		{
			return new OGetScreenLayerScreen(a->clone());
		}
	};
	class OGetScreenPath : public UnaryOpcode
	{
	public:
		OGetScreenPath(Argument *A) : UnaryOpcode(A) {}
		string toString();
		Opcode *clone()
		{
			return new OGetScreenPath(a->clone());
		}
	};
	class OGetScreenWarpReturnX : public UnaryOpcode
	{
	public:
		OGetScreenWarpReturnX(Argument *A) : UnaryOpcode(A) {}
		string toString();
		Opcode *clone()
		{
			return new OGetScreenWarpReturnX(a->clone());
		}
	};
	class OGetScreenWarpReturnY : public UnaryOpcode
	{
	public:
		OGetScreenWarpReturnY(Argument *A) : UnaryOpcode(A) {}
		string toString();
		Opcode *clone()
		{
			return new OGetScreenWarpReturnY(a->clone());
		}
	};

	class OTriggerSecretRegister : public UnaryOpcode
	{
	public:
		OTriggerSecretRegister(Argument *A) : UnaryOpcode(A) {}
		string toString();
		Opcode *clone()
		{
			return new OTriggerSecretRegister(a->clone());
		}
	};

	class OPolygonRegister : public Opcode
	{
	public:
		string toString();
		Opcode *clone()
		{
			return new OPolygonRegister();
		}
	};

	class OChangeFFCScriptRegister : public UnaryOpcode
	{
	public:
		OChangeFFCScriptRegister(Argument *A) : UnaryOpcode(A) {}
		string toString();
		Opcode *clone()
		{
			return new OChangeFFCScriptRegister(a->clone());
		}
	};



	class ONDataBaseTile : public BinaryOpcode
	{
	public:
		ONDataBaseTile(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new ONDataBaseTile(a->clone(), b->clone());
		}
	};

	class ONDataEHeight : public BinaryOpcode
	{
	public:
		ONDataEHeight(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new ONDataEHeight(a->clone(), b->clone());
		}
	};

	//one input, no return
	class ONDataFlags : public BinaryOpcode
	{
	public:
		ONDataFlags(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new ONDataFlags(a->clone(), b->clone());
		}
	};
	//one input, no return
	class ONDataFlags2 : public BinaryOpcode
	{
	public:
		ONDataFlags2(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new ONDataFlags2(a->clone(), b->clone());
		}
	};
	//one input, no return
	class ONDataWidth : public BinaryOpcode
	{
	public:
		ONDataWidth(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new ONDataWidth(a->clone(), b->clone());
		}
	};
	//one input, no return
	class ONDataHeight : public BinaryOpcode
	{
	public:
		ONDataHeight(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new ONDataHeight(a->clone(), b->clone());
		}
	};
	//one input, no return
	class ONDataTile : public BinaryOpcode
	{
	public:
		ONDataTile(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new ONDataTile(a->clone(), b->clone());
		}
	};
	//one input, no return
	class ONDataSWidth : public BinaryOpcode
	{
	public:
		ONDataSWidth(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new ONDataSWidth(a->clone(), b->clone());
		}
	};
	//one input, no return
	class ONDataSHeight : public BinaryOpcode
	{
	public:
		ONDataSHeight(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new ONDataSHeight(a->clone(), b->clone());
		}
	};
	//one input, no return
	class ONDataETile : public BinaryOpcode
	{
	public:
		ONDataETile(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new ONDataETile(a->clone(), b->clone());
		}
	};
	//one input, no return
	class ONDataEWidth : public BinaryOpcode
	{
	public:
		ONDataEWidth(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new ONDataEWidth(a->clone(), b->clone());
		}
	};
	//one input, no return
	class ONDataHP : public BinaryOpcode
	{
	public:
		ONDataHP(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new ONDataHP(a->clone(), b->clone());
		}
	};
	//one input, no return
	class ONDataFamily : public BinaryOpcode
	{
	public:
		ONDataFamily(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new ONDataFamily(a->clone(), b->clone());
		}
	};
	//one input, no return
	class ONDataCSet : public BinaryOpcode
	{
	public:
		ONDataCSet(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new ONDataCSet(a->clone(), b->clone());
		}
	};
	//one input, no return
	class ONDataAnim : public BinaryOpcode
	{
	public:
		ONDataAnim(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new ONDataAnim(a->clone(), b->clone());
		}
	};
	//one input, no return
	class ONDataEAnim : public BinaryOpcode
	{
	public:
		ONDataEAnim(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new ONDataEAnim(a->clone(), b->clone());
		}
	};
	//one input, no return
	class ONDataFramerate : public BinaryOpcode
	{
	public:
		ONDataFramerate(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new ONDataFramerate(a->clone(), b->clone());
		}
	};
	//one input, no return
	class ONDataEFramerate : public BinaryOpcode
	{
	public:
		ONDataEFramerate(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new ONDataEFramerate(a->clone(), b->clone());
		}
	};
	//one input, no return
	class ONDataTouchDamage : public BinaryOpcode
	{
	public:
		ONDataTouchDamage(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new ONDataTouchDamage(a->clone(), b->clone());
		}
	};
	//one input, no return
	class ONDataWeaponDamage : public BinaryOpcode
	{
	public:
		ONDataWeaponDamage(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new ONDataWeaponDamage(a->clone(), b->clone());
		}
	};
	//one input, no return
	class ONDataWeapon : public BinaryOpcode
	{
	public:
		ONDataWeapon(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new ONDataWeapon(a->clone(), b->clone());
		}
	};
	//one input, no return
	class ONDataRandom : public BinaryOpcode
	{
	public:
		ONDataRandom(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new ONDataRandom(a->clone(), b->clone());
		}
	};
	//one input, no return
	class ONDataHalt : public BinaryOpcode
	{
	public:
		ONDataHalt(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new ONDataHalt(a->clone(), b->clone());
		}
	};
	//one input, no return
	class ONDataStep : public BinaryOpcode
	{
	public:
		ONDataStep(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new ONDataStep(a->clone(), b->clone());
		}
	};
	//one input, no return
	class ONDataHoming : public BinaryOpcode
	{
	public:
		ONDataHoming(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new ONDataHoming(a->clone(), b->clone());
		}
	};
	//one input, no return
	class ONDataHunger : public BinaryOpcode
	{
	public:
		ONDataHunger(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new ONDataHunger(a->clone(), b->clone());
		}
	};
	//one input, no return
	class ONDataropset : public BinaryOpcode
	{
	public:
		ONDataropset(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new ONDataropset(a->clone(), b->clone());
		}
	};
	//one input, no return
	class ONDataBGSound : public BinaryOpcode
	{
	public:
		ONDataBGSound(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new ONDataBGSound(a->clone(), b->clone());
		}
	};
	//one input, no return
	class ONDataHitSound : public BinaryOpcode
	{
	public:
		ONDataHitSound(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new ONDataHitSound(a->clone(), b->clone());
		}
	};
	//one input, no return
	class ONDataDeathSound : public BinaryOpcode
	{
	public:
		ONDataDeathSound(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new ONDataDeathSound(a->clone(), b->clone());
		}
	};
	//one input, no return
	class ONDataXofs : public BinaryOpcode
	{
	public:
		ONDataXofs(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new ONDataXofs(a->clone(), b->clone());
		}
	};
	//one input, no return
	class ONDataYofs : public BinaryOpcode
	{
	public:
		ONDataYofs(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new ONDataYofs(a->clone(), b->clone());
		}
	};
	//one input, no return
	class ONDataZofs : public BinaryOpcode
	{
	public:
		ONDataZofs(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new ONDataZofs(a->clone(), b->clone());
		}
	};
	//one input, no return
	class ONDataHitXOfs : public BinaryOpcode
	{
	public:
		ONDataHitXOfs(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new ONDataHitXOfs(a->clone(), b->clone());
		}
	};
	//one input, no return
	class ONDataHYOfs : public BinaryOpcode
	{
	public:
		ONDataHYOfs(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new ONDataHYOfs(a->clone(), b->clone());
		}
	};
	//one input, no return
	class ONDataHitWidth : public BinaryOpcode
	{
	public:
		ONDataHitWidth(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new ONDataHitWidth(a->clone(), b->clone());
		}
	};
	//one input, no return
	class ONDataHitHeight : public BinaryOpcode
	{
	public:
		ONDataHitHeight(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new ONDataHitHeight(a->clone(), b->clone());
		}
	};
	//one input, no return
	class ONDataHitZ : public BinaryOpcode
	{
	public:
		ONDataHitZ(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new ONDataHitZ(a->clone(), b->clone());
		}
	};
	//one input, no return
	class ONDataTileWidth : public BinaryOpcode
	{
	public:
		ONDataTileWidth(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new ONDataTileWidth(a->clone(), b->clone());
		}
	};
	//one input, no return
	class ONDataTileHeight : public BinaryOpcode
	{
	public:
		ONDataTileHeight(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new ONDataTileHeight(a->clone(), b->clone());
		}
	};
	//one input, no return
	class ONDataWeapSprite : public BinaryOpcode
	{
	public:
		ONDataWeapSprite(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new ONDataWeapSprite(a->clone(), b->clone());
		}
	};

	//two inputs, two returns

	class ONDataScriptDef : public UnaryOpcode
	{
	public:
		ONDataScriptDef(Argument *A) : UnaryOpcode(A) {}
		string toString();
		Opcode *clone()
		{
			return new ONDataScriptDef(a->clone());
		}
	};
	//two inputs, two returns

	class ONDataDefense : public UnaryOpcode
	{
	public:
		ONDataDefense(Argument *A) : UnaryOpcode(A) {}
		string toString();
		Opcode *clone()
		{
			return new ONDataDefense(a->clone());
		}
	};
	//two inputs, two returns

	class ONDataSizeFlag : public UnaryOpcode
	{
	public:
		ONDataSizeFlag(Argument *A) : UnaryOpcode(A) {}
		string toString();
		Opcode *clone()
		{
			return new ONDataSizeFlag(a->clone());
		}
	};
	//two inputs, two returns

	class ONDatattributes : public UnaryOpcode
	{
	public:
		ONDatattributes(Argument *A) : UnaryOpcode(A) {}
		string toString();
		Opcode *clone()
		{
			return new ONDatattributes(a->clone());
		}
	};

	class ONDataSetBaseTile : public BinaryOpcode
	{
	public:
		ONDataSetBaseTile(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new ONDataSetBaseTile(a->clone(), b->clone());
		}
	};
	class ONDataSetEHeight : public BinaryOpcode
	{
	public:
		ONDataSetEHeight(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new ONDataSetEHeight(a->clone(), b->clone());
		}
	};

	class ONDataSetFlags : public BinaryOpcode
	{
	public:
		ONDataSetFlags(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new ONDataSetFlags(a->clone(), b->clone());
		}
	};
	//one input, no return
	class ONDataSetFlags2 : public BinaryOpcode
	{
	public:
		ONDataSetFlags2(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new ONDataSetFlags2(a->clone(), b->clone());
		}
	};
	//one input, no return
	class ONDataSetWidth : public BinaryOpcode
	{
	public:
		ONDataSetWidth(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new ONDataSetWidth(a->clone(), b->clone());
		}
	};
	//one input, no return
	class ONDataSetHeight : public BinaryOpcode
	{
	public:
		ONDataSetHeight(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new ONDataSetHeight(a->clone(), b->clone());
		}
	};
	//one input, no return
	class ONDataSetTile : public BinaryOpcode
	{
	public:
		ONDataSetTile(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new ONDataSetTile(a->clone(), b->clone());
		}
	};
	//one input, no return
	class ONDataSetSWidth : public BinaryOpcode
	{
	public:
		ONDataSetSWidth(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new ONDataSetSWidth(a->clone(), b->clone());
		}
	};
	//one input, no return
	class ONDataSetSHeight : public BinaryOpcode
	{
	public:
		ONDataSetSHeight(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new ONDataSetSHeight(a->clone(), b->clone());
		}
	};
	//one input, no return
	class ONDataSetETile : public BinaryOpcode
	{
	public:
		ONDataSetETile(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new ONDataSetETile(a->clone(), b->clone());
		}
	};
	//one input, no return
	class ONDataSetEWidth : public BinaryOpcode
	{
	public:
		ONDataSetEWidth(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new ONDataSetEWidth(a->clone(), b->clone());
		}
	};
	//one input, no return
	class ONDataSetHP : public BinaryOpcode
	{
	public:
		ONDataSetHP(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new ONDataSetHP(a->clone(), b->clone());
		}
	};
	//one input, no return
	class ONDataSetFamily : public BinaryOpcode
	{
	public:
		ONDataSetFamily(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new ONDataSetFamily(a->clone(), b->clone());
		}
	};
	//one input, no return
	class ONDataSetCSet : public BinaryOpcode
	{
	public:
		ONDataSetCSet(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new ONDataSetCSet(a->clone(), b->clone());
		}
	};
	//one input, no return
	class ONDataSetAnim : public BinaryOpcode
	{
	public:
		ONDataSetAnim(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new ONDataSetAnim(a->clone(), b->clone());
		}
	};
	//one input, no return
	class ONDataSetEAnim : public BinaryOpcode
	{
	public:
		ONDataSetEAnim(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new ONDataSetEAnim(a->clone(), b->clone());
		}
	};
	//one input, no return
	class ONDataSetFramerate : public BinaryOpcode
	{
	public:
		ONDataSetFramerate(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new ONDataSetFramerate(a->clone(), b->clone());
		}
	};
	//one input, no return
	class ONDataSetEFramerate : public BinaryOpcode
	{
	public:
		ONDataSetEFramerate(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new ONDataSetEFramerate(a->clone(), b->clone());
		}
	};
	//one input, no return
	class ONDataSetTouchDamage : public BinaryOpcode
	{
	public:
		ONDataSetTouchDamage(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new ONDataSetTouchDamage(a->clone(), b->clone());
		}
	};
	//one input, no return
	class ONDataSetWeaponDamage : public BinaryOpcode
	{
	public:
		ONDataSetWeaponDamage(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new ONDataSetWeaponDamage(a->clone(), b->clone());
		}
	};
	//one input, no return
	class ONDataSetWeapon : public BinaryOpcode
	{
	public:
		ONDataSetWeapon(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new ONDataSetWeapon(a->clone(), b->clone());
		}
	};
	//one input, no return
	class ONDataSetRandom : public BinaryOpcode
	{
	public:
		ONDataSetRandom(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new ONDataSetRandom(a->clone(), b->clone());
		}
	};
	//one input, no return
	class ONDataSetHalt : public BinaryOpcode
	{
	public:
		ONDataSetHalt(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new ONDataSetHalt(a->clone(), b->clone());
		}
	};
	//one input, no return
	class ONDataSetStep : public BinaryOpcode
	{
	public:
		ONDataSetStep(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new ONDataSetStep(a->clone(), b->clone());
		}
	};
	//one input, no return
	class ONDataSetHoming : public BinaryOpcode
	{
	public:
		ONDataSetHoming(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new ONDataSetHoming(a->clone(), b->clone());
		}
	};
	//one input, no return
	class ONDataSetHunger : public BinaryOpcode
	{
	public:
		ONDataSetHunger(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new ONDataSetHunger(a->clone(), b->clone());
		}
	};
	//one input, no return
	class ONDataSetropset : public BinaryOpcode
	{
	public:
		ONDataSetropset(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new ONDataSetropset(a->clone(), b->clone());
		}
	};
	//one input, no return
	class ONDataSetHitSound : public BinaryOpcode
	{
	public:
		ONDataSetHitSound(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new ONDataSetHitSound(a->clone(), b->clone());
		}
	};

	//one input, no return
	class ONDataSetBGSound : public BinaryOpcode
	{
	public:
		ONDataSetBGSound(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new ONDataSetBGSound(a->clone(), b->clone());
		}
	};
	//one input, no return
	class ONDataSetDeathSound : public BinaryOpcode
	{
	public:
		ONDataSetDeathSound(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new ONDataSetDeathSound(a->clone(), b->clone());
		}
	};
	//one input, no return
	class ONDataSetXofs : public BinaryOpcode
	{
	public:
		ONDataSetXofs(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new ONDataSetXofs(a->clone(), b->clone());
		}
	};
	//one input, no return
	class ONDataSetYofs : public BinaryOpcode
	{
	public:
		ONDataSetYofs(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new ONDataSetYofs(a->clone(), b->clone());
		}
	};
	//one input, no return
	class ONDataSetZofs : public BinaryOpcode
	{
	public:
		ONDataSetZofs(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new ONDataSetZofs(a->clone(), b->clone());
		}
	};
	//one input, no return
	class ONDataSetHitXOfs : public BinaryOpcode
	{
	public:
		ONDataSetHitXOfs(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new ONDataSetHitXOfs(a->clone(), b->clone());
		}
	};
	//one input, no return
	class ONDataSetHYOfs : public BinaryOpcode
	{
	public:
		ONDataSetHYOfs(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new ONDataSetHYOfs(a->clone(), b->clone());
		}
	};
	//one input, no return
	class ONDataSetHitWidth : public BinaryOpcode
	{
	public:
		ONDataSetHitWidth(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new ONDataSetHitWidth(a->clone(), b->clone());
		}
	};
	//one input, no return
	class ONDataSetHitHeight : public BinaryOpcode
	{
	public:
		ONDataSetHitHeight(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new ONDataSetHitHeight(a->clone(), b->clone());
		}
	};
	//one input, no return
	class ONDataSetHitZ : public BinaryOpcode
	{
	public:
		ONDataSetHitZ(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new ONDataSetHitZ(a->clone(), b->clone());
		}
	};
	//one input, no return
	class ONDataSetTileWidth : public BinaryOpcode
	{
	public:
		ONDataSetTileWidth(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new ONDataSetTileWidth(a->clone(), b->clone());
		}
	};
	//one input, no return
	class ONDataSetTileHeight : public BinaryOpcode
	{
	public:
		ONDataSetTileHeight(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new ONDataSetTileHeight(a->clone(), b->clone());
		}
	};
	//one input, no return
	class ONDataSetWeapSprite : public BinaryOpcode
	{
	public:
		ONDataSetWeapSprite(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new ONDataSetWeapSprite(a->clone(), b->clone());
		}
	};

	//ComboData

	class OCDataBlockEnemy : public BinaryOpcode
	{
	public:
		OCDataBlockEnemy(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OCDataBlockEnemy(a->clone(), b->clone());
		}
	};
	class OCDataBlockHole : public BinaryOpcode
	{
	public:
		OCDataBlockHole(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OCDataBlockHole(a->clone(), b->clone());
		}
	};
	class OCDataBlockTrig : public BinaryOpcode
	{
	public:
		OCDataBlockTrig(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OCDataBlockTrig(a->clone(), b->clone());
		}
	};
	class OCDataConveyX : public BinaryOpcode
	{
	public:
		OCDataConveyX(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OCDataConveyX(a->clone(), b->clone());
		}
	};
	class OCDataConveyY : public BinaryOpcode
	{
	public:
		OCDataConveyY(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OCDataConveyY(a->clone(), b->clone());
		}
	};
	class OCDataCreateNPC : public BinaryOpcode
	{
	public:
		OCDataCreateNPC(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OCDataCreateNPC(a->clone(), b->clone());
		}
	};
	class OCDataCreateEnemW : public BinaryOpcode
	{
	public:
		OCDataCreateEnemW(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OCDataCreateEnemW(a->clone(), b->clone());
		}
	};
	class OCDataCreateEnemC : public BinaryOpcode
	{
	public:
		OCDataCreateEnemC(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OCDataCreateEnemC(a->clone(), b->clone());
		}
	};
	class OCDataDirch : public BinaryOpcode
	{
	public:
		OCDataDirch(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OCDataDirch(a->clone(), b->clone());
		}
	};
	class OCDataDistTiles : public BinaryOpcode
	{
	public:
		OCDataDistTiles(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OCDataDistTiles(a->clone(), b->clone());
		}
	};
	class OCDataDiveItem : public BinaryOpcode
	{
	public:
		OCDataDiveItem(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OCDataDiveItem(a->clone(), b->clone());
		}
	};
	class OCDataAttrib : public BinaryOpcode
	{
	public:
		OCDataAttrib(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OCDataAttrib(a->clone(), b->clone());
		}
	};
	class OCDataDecoTile : public BinaryOpcode
	{
	public:
		OCDataDecoTile(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OCDataDecoTile(a->clone(), b->clone());
		}
	};
	class OCDataDock : public BinaryOpcode
	{
	public:
		OCDataDock(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OCDataDock(a->clone(), b->clone());
		}
	};
	class OCDataFairy : public BinaryOpcode
	{
	public:
		OCDataFairy(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OCDataFairy(a->clone(), b->clone());
		}
	};
	class OCDataDecoType : public BinaryOpcode
	{
	public:
		OCDataDecoType(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OCDataDecoType(a->clone(), b->clone());
		}
	};
	class OCDataHookshotGrab : public BinaryOpcode
	{
	public:
		OCDataHookshotGrab(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OCDataHookshotGrab(a->clone(), b->clone());
		}
	};
	class OCDataLockBlock : public BinaryOpcode
	{
	public:
		OCDataLockBlock(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OCDataLockBlock(a->clone(), b->clone());
		}
	};
	class OCDataLockBlockChange : public BinaryOpcode
	{
	public:
		OCDataLockBlockChange(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OCDataLockBlockChange(a->clone(), b->clone());
		}
	};
	class OCDataMagicMirror : public BinaryOpcode
	{
	public:
		OCDataMagicMirror(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OCDataMagicMirror(a->clone(), b->clone());
		}
	};
	class OCDataModHP : public BinaryOpcode
	{
	public:
		OCDataModHP(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OCDataModHP(a->clone(), b->clone());
		}
	};
	class OCDataModHPDelay : public BinaryOpcode
	{
	public:
		OCDataModHPDelay(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OCDataModHPDelay(a->clone(), b->clone());
		}
	};
	class OCDataModHpType : public BinaryOpcode
	{
	public:
		OCDataModHpType(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OCDataModHpType(a->clone(), b->clone());
		}
	};
	class OCDataModMP : public BinaryOpcode
	{
	public:
		OCDataModMP(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OCDataModMP(a->clone(), b->clone());
		}
	};
	class OCDataMpdMPDelay : public BinaryOpcode
	{
	public:
		OCDataMpdMPDelay(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OCDataMpdMPDelay(a->clone(), b->clone());
		}
	};
	class OCDataModMPType : public BinaryOpcode
	{
	public:
		OCDataModMPType(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OCDataModMPType(a->clone(), b->clone());
		}
	};
	class OCDataNoPush : public BinaryOpcode
	{
	public:
		OCDataNoPush(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OCDataNoPush(a->clone(), b->clone());
		}
	};
	class OCDataOverhead : public BinaryOpcode
	{
	public:
		OCDataOverhead(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OCDataOverhead(a->clone(), b->clone());
		}
	};
	class OCDataEnemyLoc : public BinaryOpcode
	{
	public:
		OCDataEnemyLoc(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OCDataEnemyLoc(a->clone(), b->clone());
		}
	};
	class OCDataPushDir : public BinaryOpcode
	{
	public:
		OCDataPushDir(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OCDataPushDir(a->clone(), b->clone());
		}
	};
	class OCDataPushWeight : public BinaryOpcode
	{
	public:
		OCDataPushWeight(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OCDataPushWeight(a->clone(), b->clone());
		}
	};
	class OCDataPushWait : public BinaryOpcode
	{
	public:
		OCDataPushWait(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OCDataPushWait(a->clone(), b->clone());
		}
	};
	class OCDataPushed : public BinaryOpcode
	{
	public:
		OCDataPushed(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OCDataPushed(a->clone(), b->clone());
		}
	};
	class OCDataRaft : public BinaryOpcode
	{
	public:
		OCDataRaft(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OCDataRaft(a->clone(), b->clone());
		}
	};
	class OCDataResetRoom : public BinaryOpcode
	{
	public:
		OCDataResetRoom(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OCDataResetRoom(a->clone(), b->clone());
		}
	};
	class OCDataSavePoint : public BinaryOpcode
	{
	public:
		OCDataSavePoint(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OCDataSavePoint(a->clone(), b->clone());
		}
	};
	class OCDataFreeezeScreen : public BinaryOpcode
	{
	public:
		OCDataFreeezeScreen(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OCDataFreeezeScreen(a->clone(), b->clone());
		}
	};
	class OCDataSecretCombo : public BinaryOpcode
	{
	public:
		OCDataSecretCombo(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OCDataSecretCombo(a->clone(), b->clone());
		}
	};
	class OCDataSingular : public BinaryOpcode
	{
	public:
		OCDataSingular(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OCDataSingular(a->clone(), b->clone());
		}
	};
	class OCDataSlowMove : public BinaryOpcode
	{
	public:
		OCDataSlowMove(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OCDataSlowMove(a->clone(), b->clone());
		}
	};
	class OCDataStatue : public BinaryOpcode
	{
	public:
		OCDataStatue(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OCDataStatue(a->clone(), b->clone());
		}
	};
	class OCDataStepType : public BinaryOpcode
	{
	public:
		OCDataStepType(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OCDataStepType(a->clone(), b->clone());
		}
	};
	class OCDataSteoChange : public BinaryOpcode
	{
	public:
		OCDataSteoChange(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OCDataSteoChange(a->clone(), b->clone());
		}
	};
	class OCDataStrikeRem : public BinaryOpcode
	{
	public:
		OCDataStrikeRem(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OCDataStrikeRem(a->clone(), b->clone());
		}
	};
	class OCDataStrikeRemType : public BinaryOpcode
	{
	public:
		OCDataStrikeRemType(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OCDataStrikeRemType(a->clone(), b->clone());
		}
	};
	class OCDataStrikeChange : public BinaryOpcode
	{
	public:
		OCDataStrikeChange(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OCDataStrikeChange(a->clone(), b->clone());
		}
	};
	class OCDataStrikeChangeItem : public BinaryOpcode
	{
	public:
		OCDataStrikeChangeItem(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OCDataStrikeChangeItem(a->clone(), b->clone());
		}
	};
	class OCDataTouchItem : public BinaryOpcode
	{
	public:
		OCDataTouchItem(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OCDataTouchItem(a->clone(), b->clone());
		}
	};
	class OCDataTouchStairs : public BinaryOpcode
	{
	public:
		OCDataTouchStairs(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OCDataTouchStairs(a->clone(), b->clone());
		}
	};
	class OCDataTriggerType : public BinaryOpcode
	{
	public:
		OCDataTriggerType(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OCDataTriggerType(a->clone(), b->clone());
		}
	};
	class OCDataTriggerSens : public BinaryOpcode
	{
	public:
		OCDataTriggerSens(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OCDataTriggerSens(a->clone(), b->clone());
		}
	};
	class OCDataWarpType : public BinaryOpcode
	{
	public:
		OCDataWarpType(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OCDataWarpType(a->clone(), b->clone());
		}
	};
	class OCDataWarpSens : public BinaryOpcode
	{
	public:
		OCDataWarpSens(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OCDataWarpSens(a->clone(), b->clone());
		}
	};
	class OCDataWarpDirect : public BinaryOpcode
	{
	public:
		OCDataWarpDirect(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OCDataWarpDirect(a->clone(), b->clone());
		}
	};
	class OCDataWarpLoc : public BinaryOpcode
	{
	public:
		OCDataWarpLoc(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OCDataWarpLoc(a->clone(), b->clone());
		}
	};
	class OCDataWater : public BinaryOpcode
	{
	public:
		OCDataWater(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OCDataWater(a->clone(), b->clone());
		}
	};

	class OCDataWinGame : public BinaryOpcode
	{
	public:
		OCDataWinGame(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OCDataWinGame(a->clone(), b->clone());
		}
	};

	class OCDataWhistle : public BinaryOpcode
	{
	public:
		OCDataWhistle(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OCDataWhistle(a->clone(), b->clone());
		}
	};
	class OCDataWeapBlockLevel : public BinaryOpcode
	{
	public:
		OCDataWeapBlockLevel(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OCDataWeapBlockLevel(a->clone(), b->clone());
		}
	};
	class OCDataTile : public BinaryOpcode
	{
	public:
		OCDataTile(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OCDataTile(a->clone(), b->clone());
		}
	};
	class OCDataFlip : public BinaryOpcode
	{
	public:
		OCDataFlip(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OCDataFlip(a->clone(), b->clone());
		}
	};
	class OCDataWalkability : public BinaryOpcode
	{
	public:
		OCDataWalkability(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OCDataWalkability(a->clone(), b->clone());
		}
	};
	class OCDataType : public BinaryOpcode
	{
	public:
		OCDataType(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OCDataType(a->clone(), b->clone());
		}
	};
	class OCDataCSets : public BinaryOpcode
	{
	public:
		OCDataCSets(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OCDataCSets(a->clone(), b->clone());
		}
	};
	class OCDataFoo : public BinaryOpcode
	{
	public:
		OCDataFoo(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OCDataFoo(a->clone(), b->clone());
		}
	};
	class OCDataFrames : public BinaryOpcode
	{
	public:
		OCDataFrames(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OCDataFrames(a->clone(), b->clone());
		}
	};
	class OCDataSpeed : public BinaryOpcode
	{
	public:
		OCDataSpeed(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OCDataSpeed(a->clone(), b->clone());
		}
	};
	class OCDataNext : public BinaryOpcode
	{
	public:
		OCDataNext(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OCDataNext(a->clone(), b->clone());
		}
	};
	class OCDataNextCSet : public BinaryOpcode
	{
	public:
		OCDataNextCSet(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OCDataNextCSet(a->clone(), b->clone());
		}
	};
	class OCDataFlag : public BinaryOpcode
	{
	public:
		OCDataFlag(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OCDataFlag(a->clone(), b->clone());
		}
	};
	class OCDataSkipAnim : public BinaryOpcode
	{
	public:
		OCDataSkipAnim(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OCDataSkipAnim(a->clone(), b->clone());
		}
	};
	class OCDataTimer : public BinaryOpcode
	{
	public:
		OCDataTimer(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OCDataTimer(a->clone(), b->clone());
		}
	};
	class OCDataAnimY : public BinaryOpcode
	{
	public:
		OCDataAnimY(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OCDataAnimY(a->clone(), b->clone());
		}
	};
	class OCDataAnimFlags : public BinaryOpcode
	{
	public:
		OCDataAnimFlags(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OCDataAnimFlags(a->clone(), b->clone());
		}
	};
	class OCDataBlockWeapon : public UnaryOpcode
	{
	public:
		OCDataBlockWeapon(Argument *A) : UnaryOpcode(A) {}
		string toString();
		Opcode *clone()
		{
			return new OCDataBlockWeapon(a->clone());
		}
	};
	class OCDataExpansion : public UnaryOpcode
	{
	public:
		OCDataExpansion(Argument *A) : UnaryOpcode(A) {}
		string toString();
		Opcode *clone()
		{
			return new OCDataExpansion(a->clone());
		}
	};
	class OCDataStrikeWeapon : public UnaryOpcode
	{
	public:
		OCDataStrikeWeapon(Argument *A) : UnaryOpcode(A) {}
		string toString();
		Opcode *clone()
		{
			return new OCDataStrikeWeapon(a->clone());
		}
	};


	class OCSetDataBlockEnemy : public BinaryOpcode
	{
	public:
		OCSetDataBlockEnemy(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OCSetDataBlockEnemy(a->clone(), b->clone());
		}
	};
	class OCSetDataBlockHole : public BinaryOpcode
	{
	public:
		OCSetDataBlockHole(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OCSetDataBlockHole(a->clone(), b->clone());
		}
	};
	class OCSetDataBlockTrig : public BinaryOpcode
	{
	public:
		OCSetDataBlockTrig(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OCSetDataBlockTrig(a->clone(), b->clone());
		}
	};
	class OCSetDataConveyX : public BinaryOpcode
	{
	public:
		OCSetDataConveyX(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OCSetDataConveyX(a->clone(), b->clone());
		}
	};
	class OCSetDataConveyY : public BinaryOpcode
	{
	public:
		OCSetDataConveyY(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OCSetDataConveyY(a->clone(), b->clone());
		}
	};
	class OCSetDataCreateNPC : public BinaryOpcode
	{
	public:
		OCSetDataCreateNPC(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OCSetDataCreateNPC(a->clone(), b->clone());
		}
	};
	class OCSetDataCreateEnemW : public BinaryOpcode
	{
	public:
		OCSetDataCreateEnemW(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OCSetDataCreateEnemW(a->clone(), b->clone());
		}
	};
	class OCSetDataCreateEnemC : public BinaryOpcode
	{
	public:
		OCSetDataCreateEnemC(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OCSetDataCreateEnemC(a->clone(), b->clone());
		}
	};
	class OCSetDataDirch : public BinaryOpcode
	{
	public:
		OCSetDataDirch(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OCSetDataDirch(a->clone(), b->clone());
		}
	};
	class OCSetDataDistTiles : public BinaryOpcode
	{
	public:
		OCSetDataDistTiles(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OCSetDataDistTiles(a->clone(), b->clone());
		}
	};
	class OCSetDataDiveItem : public BinaryOpcode
	{
	public:
		OCSetDataDiveItem(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OCSetDataDiveItem(a->clone(), b->clone());
		}
	};
	class OCSetDataAttrib : public BinaryOpcode
	{
	public:
		OCSetDataAttrib(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OCSetDataAttrib(a->clone(), b->clone());
		}
	};
	class OCSetDataDecoTile : public BinaryOpcode
	{
	public:
		OCSetDataDecoTile(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OCSetDataDecoTile(a->clone(), b->clone());
		}
	};
	class OCSetDataDock : public BinaryOpcode
	{
	public:
		OCSetDataDock(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OCSetDataDock(a->clone(), b->clone());
		}
	};
	class OCSetDataFairy : public BinaryOpcode
	{
	public:
		OCSetDataFairy(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OCSetDataFairy(a->clone(), b->clone());
		}
	};
	class OCSetDataDecoType : public BinaryOpcode
	{
	public:
		OCSetDataDecoType(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OCSetDataDecoType(a->clone(), b->clone());
		}
	};
	class OCSetDataHookshotGrab : public BinaryOpcode
	{
	public:
		OCSetDataHookshotGrab(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OCSetDataHookshotGrab(a->clone(), b->clone());
		}
	};
	class OCSetDataLockBlock : public BinaryOpcode
	{
	public:
		OCSetDataLockBlock(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OCSetDataLockBlock(a->clone(), b->clone());
		}
	};
	class OCSetDataLockBlockChange : public BinaryOpcode
	{
	public:
		OCSetDataLockBlockChange(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OCSetDataLockBlockChange(a->clone(), b->clone());
		}
	};
	class OCSetDataMagicMirror : public BinaryOpcode
	{
	public:
		OCSetDataMagicMirror(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OCSetDataMagicMirror(a->clone(), b->clone());
		}
	};
	class OCSetDataModHP : public BinaryOpcode
	{
	public:
		OCSetDataModHP(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OCSetDataModHP(a->clone(), b->clone());
		}
	};
	class OCSetDataModHPDelay : public BinaryOpcode
	{
	public:
		OCSetDataModHPDelay(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OCSetDataModHPDelay(a->clone(), b->clone());
		}
	};
	class OCSetDataModHpType : public BinaryOpcode
	{
	public:
		OCSetDataModHpType(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OCSetDataModHpType(a->clone(), b->clone());
		}
	};
	class OCSetDataModMP : public BinaryOpcode
	{
	public:
		OCSetDataModMP(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OCSetDataModMP(a->clone(), b->clone());
		}
	};
	class OCSetDataMpdMPDelay : public BinaryOpcode
	{
	public:
		OCSetDataMpdMPDelay(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OCSetDataMpdMPDelay(a->clone(), b->clone());
		}
	};
	class OCSetDataModMPType : public BinaryOpcode
	{
	public:
		OCSetDataModMPType(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OCSetDataModMPType(a->clone(), b->clone());
		}
	};
	class OCSetDataNoPush : public BinaryOpcode
	{
	public:
		OCSetDataNoPush(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OCSetDataNoPush(a->clone(), b->clone());
		}
	};
	class OCSetDataOverhead : public BinaryOpcode
	{
	public:
		OCSetDataOverhead(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OCSetDataOverhead(a->clone(), b->clone());
		}
	};
	class OCSetDataEnemyLoc : public BinaryOpcode
	{
	public:
		OCSetDataEnemyLoc(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OCSetDataEnemyLoc(a->clone(), b->clone());
		}
	};
	class OCSetDataPushDir : public BinaryOpcode
	{
	public:
		OCSetDataPushDir(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OCSetDataPushDir(a->clone(), b->clone());
		}
	};
	class OCSetDataPushWeight : public BinaryOpcode
	{
	public:
		OCSetDataPushWeight(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OCSetDataPushWeight(a->clone(), b->clone());
		}
	};
	class OCSetDataPushWait : public BinaryOpcode
	{
	public:
		OCSetDataPushWait(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OCSetDataPushWait(a->clone(), b->clone());
		}
	};
	class OCSetDataPushed : public BinaryOpcode
	{
	public:
		OCSetDataPushed(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OCSetDataPushed(a->clone(), b->clone());
		}
	};
	class OCSetDataRaft : public BinaryOpcode
	{
	public:
		OCSetDataRaft(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OCSetDataRaft(a->clone(), b->clone());
		}
	};
	class OCSetDataResetRoom : public BinaryOpcode
	{
	public:
		OCSetDataResetRoom(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OCSetDataResetRoom(a->clone(), b->clone());
		}
	};
	class OCSetDataSavePoint : public BinaryOpcode
	{
	public:
		OCSetDataSavePoint(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OCSetDataSavePoint(a->clone(), b->clone());
		}
	};
	class OCSetDataFreeezeScreen : public BinaryOpcode
	{
	public:
		OCSetDataFreeezeScreen(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OCSetDataFreeezeScreen(a->clone(), b->clone());
		}
	};
	class OCSetDataSecretCombo : public BinaryOpcode
	{
	public:
		OCSetDataSecretCombo(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OCSetDataSecretCombo(a->clone(), b->clone());
		}
	};
	class OCSetDataSingular : public BinaryOpcode
	{
	public:
		OCSetDataSingular(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OCSetDataSingular(a->clone(), b->clone());
		}
	};
	class OCSetDataSlowMove : public BinaryOpcode
	{
	public:
		OCSetDataSlowMove(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OCSetDataSlowMove(a->clone(), b->clone());
		}
	};
	class OCSetDataStatue : public BinaryOpcode
	{
	public:
		OCSetDataStatue(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OCSetDataStatue(a->clone(), b->clone());
		}
	};
	class OCSetDataStepType : public BinaryOpcode
	{
	public:
		OCSetDataStepType(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OCSetDataStepType(a->clone(), b->clone());
		}
	};
	class OCSetDataSteoChange : public BinaryOpcode
	{
	public:
		OCSetDataSteoChange(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OCSetDataSteoChange(a->clone(), b->clone());
		}
	};
	class OCSetDataStrikeRem : public BinaryOpcode
	{
	public:
		OCSetDataStrikeRem(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OCSetDataStrikeRem(a->clone(), b->clone());
		}
	};
	class OCSetDataStrikeRemType : public BinaryOpcode
	{
	public:
		OCSetDataStrikeRemType(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OCSetDataStrikeRemType(a->clone(), b->clone());
		}
	};
	class OCSetDataStrikeChange : public BinaryOpcode
	{
	public:
		OCSetDataStrikeChange(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OCSetDataStrikeChange(a->clone(), b->clone());
		}
	};
	class OCSetDataStrikeChangeItem : public BinaryOpcode
	{
	public:
		OCSetDataStrikeChangeItem(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OCSetDataStrikeChangeItem(a->clone(), b->clone());
		}
	};
	class OCSetDataTouchItem : public BinaryOpcode
	{
	public:
		OCSetDataTouchItem(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OCSetDataTouchItem(a->clone(), b->clone());
		}
	};
	class OCSetDataTouchStairs : public BinaryOpcode
	{
	public:
		OCSetDataTouchStairs(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OCSetDataTouchStairs(a->clone(), b->clone());
		}
	};
	class OCSetDataTriggerType : public BinaryOpcode
	{
	public:
		OCSetDataTriggerType(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OCSetDataTriggerType(a->clone(), b->clone());
		}
	};
	class OCSetDataTriggerSens : public BinaryOpcode
	{
	public:
		OCSetDataTriggerSens(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OCSetDataTriggerSens(a->clone(), b->clone());
		}
	};
	class OCSetDataWarpType : public BinaryOpcode
	{
	public:
		OCSetDataWarpType(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OCSetDataWarpType(a->clone(), b->clone());
		}
	};
	class OCSetDataWarpSens : public BinaryOpcode
	{
	public:
		OCSetDataWarpSens(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OCSetDataWarpSens(a->clone(), b->clone());
		}
	};
	class OCSetDataWarpDirect : public BinaryOpcode
	{
	public:
		OCSetDataWarpDirect(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OCSetDataWarpDirect(a->clone(), b->clone());
		}
	};
	class OCSetDataWarpLoc : public BinaryOpcode
	{
	public:
		OCSetDataWarpLoc(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OCSetDataWarpLoc(a->clone(), b->clone());
		}
	};
	class OCSetDataWater : public BinaryOpcode
	{
	public:
		OCSetDataWater(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OCSetDataWater(a->clone(), b->clone());
		}
	};
	class OCSetDataWhistle : public BinaryOpcode
	{
	public:
		OCSetDataWhistle(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OCSetDataWhistle(a->clone(), b->clone());
		}
	};
	class OCSetDataWeapBlockLevel : public BinaryOpcode
	{
	public:
		OCSetDataWeapBlockLevel(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OCSetDataWeapBlockLevel(a->clone(), b->clone());
		}
	};
	class OCSetDataTile : public BinaryOpcode
	{
	public:
		OCSetDataTile(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OCSetDataTile(a->clone(), b->clone());
		}
	};
	class OCSetDataFlip : public BinaryOpcode
	{
	public:
		OCSetDataFlip(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OCSetDataFlip(a->clone(), b->clone());
		}
	};
	class OCSetDataWalkability : public BinaryOpcode
	{
	public:
		OCSetDataWalkability(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OCSetDataWalkability(a->clone(), b->clone());
		}
	};
	class OCSetDataType : public BinaryOpcode
	{
	public:
		OCSetDataType(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OCSetDataType(a->clone(), b->clone());
		}
	};
	class OCSetDataCSets : public BinaryOpcode
	{
	public:
		OCSetDataCSets(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OCSetDataCSets(a->clone(), b->clone());
		}
	};
	class OCSetDataFoo : public BinaryOpcode
	{
	public:
		OCSetDataFoo(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OCSetDataFoo(a->clone(), b->clone());
		}
	};
	class OCSetDataFrames : public BinaryOpcode
	{
	public:
		OCSetDataFrames(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OCSetDataFrames(a->clone(), b->clone());
		}
	};
	class OCSetDataSpeed : public BinaryOpcode
	{
	public:
		OCSetDataSpeed(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OCSetDataSpeed(a->clone(), b->clone());
		}
	};
	class OCSetDataNext : public BinaryOpcode
	{
	public:
		OCSetDataNext(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OCSetDataNext(a->clone(), b->clone());
		}
	};
	class OCSetDataNextCSet : public BinaryOpcode
	{
	public:
		OCSetDataNextCSet(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OCSetDataNextCSet(a->clone(), b->clone());
		}
	};
	class OCSetDataFlag : public BinaryOpcode
	{
	public:
		OCSetDataFlag(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OCSetDataFlag(a->clone(), b->clone());
		}
	};
	class OCSetDataSkipAnim : public BinaryOpcode
	{
	public:
		OCSetDataSkipAnim(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OCSetDataSkipAnim(a->clone(), b->clone());
		}
	};

	class OCDataLadderPass : public BinaryOpcode
	{
	public:
		OCDataLadderPass(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OCDataLadderPass(a->clone(), b->clone());
		}
	};

	class OCSetDataLadderPass : public BinaryOpcode
	{
	public:
		OCSetDataLadderPass(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OCSetDataLadderPass(a->clone(), b->clone());
		}
	};
	class OCSetDataTimer : public BinaryOpcode
	{
	public:
		OCSetDataTimer(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OCSetDataTimer(a->clone(), b->clone());
		}
	};
	class OCSetDataAnimY : public BinaryOpcode
	{
	public:
		OCSetDataAnimY(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OCSetDataAnimY(a->clone(), b->clone());
		}
	};
	class OCSetDataAnimFlags : public BinaryOpcode
	{
	public:
		OCSetDataAnimFlags(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OCSetDataAnimFlags(a->clone(), b->clone());
		}
	};
	class OCSetDataBlockWeapon : public UnaryOpcode
	{
	public:
		OCSetDataBlockWeapon(Argument *A) : UnaryOpcode(A) {}
		string toString();
		Opcode *clone()
		{
			return new OCSetDataBlockWeapon(a->clone());
		}
	};
	class OCSetDataExpansion : public UnaryOpcode
	{
	public:
		OCSetDataExpansion(Argument *A) : UnaryOpcode(A) {}
		string toString();
		Opcode *clone()
		{
			return new OCSetDataExpansion(a->clone());
		}
	};
	class OCSetDataStrikeWeapon : public UnaryOpcode
	{
	public:
		OCSetDataStrikeWeapon(Argument *A) : UnaryOpcode(A) {}
		string toString();
		Opcode *clone()
		{
			return new OCSetDataStrikeWeapon(a->clone());
		}
	};

	class OCSetDataWinGame : public BinaryOpcode
	{
	public:
		OCSetDataWinGame(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OCSetDataWinGame(a->clone(), b->clone());
		}
	};

	//SpriteData
	class OSDataTile : public BinaryOpcode
	{
	public:
		OSDataTile(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OSDataTile(a->clone(), b->clone());
		}
	};

	class OSDataMisc : public BinaryOpcode
	{
	public:
		OSDataMisc(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OSDataMisc(a->clone(), b->clone());
		}
	};

	class OSDataCSets : public BinaryOpcode
	{
	public:
		OSDataCSets(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OSDataCSets(a->clone(), b->clone());
		}
	};

	class OSDataFrames : public BinaryOpcode
	{
	public:
		OSDataFrames(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OSDataFrames(a->clone(), b->clone());
		}
	};

	class OSDataSpeed : public BinaryOpcode
	{
	public:
		OSDataSpeed(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OSDataSpeed(a->clone(), b->clone());
		}
	};
	class OSDataType : public BinaryOpcode
	{
	public:
		OSDataType(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OSDataType(a->clone(), b->clone());
		}
	};

	class OSSetDataTile : public BinaryOpcode
	{
	public:
		OSSetDataTile(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OSSetDataTile(a->clone(), b->clone());
		}
	};

	class OSSetDataMisc : public BinaryOpcode
	{
	public:
		OSSetDataMisc(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OSSetDataMisc(a->clone(), b->clone());
		}
	};

	class OSSetDataCSets : public BinaryOpcode
	{
	public:
		OSSetDataCSets(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OSSetDataCSets(a->clone(), b->clone());
		}
	};

	class OSSetDataFrames : public BinaryOpcode
	{
	public:
		OSSetDataFrames(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OSSetDataFrames(a->clone(), b->clone());
		}
	};

	class OSSetDataSpeed : public BinaryOpcode
	{
	public:
		OSSetDataSpeed(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OSSetDataSpeed(a->clone(), b->clone());
		}
	};
	class OSSetDataType : public BinaryOpcode
	{
	public:
		OSSetDataType(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OSSetDataType(a->clone(), b->clone());
		}
	};

	//Continue Screen


	class OSSetContinueScreen : public BinaryOpcode
	{
	public:
		OSSetContinueScreen(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OSSetContinueScreen(a->clone(), b->clone());
		}
	};
	class OSSetContinueString : public BinaryOpcode
	{
	public:
		OSSetContinueString(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		string toString();
		Opcode *clone()
		{
			return new OSSetContinueString	(a->clone(), b->clone());
		}
	};

	//Visual effects with one bool input


	class OWavyR : public UnaryOpcode
	{
	public:
		OWavyR(Argument *A) : UnaryOpcode(A) {}
		string toString();
		Opcode *clone()
		{
			return new OWavyR(a->clone());
		}
	};

	class OZapR : public UnaryOpcode
	{
	public:
		OZapR(Argument *A) : UnaryOpcode(A) {}
		string toString();
		Opcode *clone()
		{
			return new OZapR(a->clone());
		}
	};

	class OGreyscaleR : public UnaryOpcode
	{
	public:
		OGreyscaleR(Argument *A) : UnaryOpcode(A) {}
		string toString();
		Opcode *clone()
		{
			return new OGreyscaleR(a->clone());
		}
	};
}

#endif

