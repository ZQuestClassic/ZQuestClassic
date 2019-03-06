
    
    { jwin_text_proc,       6+10,   29+20,   24,    36,   0,        0,       0,       0,          0,             0, (void *) "D0:", NULL, NULL },
    { jwin_text_proc,       6+10,   47+20,   24,    36,   0,        0,       0,       0,          0,             0, (void *) "D1:", NULL, NULL },
    { jwin_text_proc,       6+10,   65+20,   24,    36,   0,        0,       0,       0,          0,             0, (void *) "D2:", NULL, NULL },
    { jwin_text_proc,       6+10,   83+20,   24,    36,   0,        0,       0,       0,          0,             0, (void *) "D3:", NULL, NULL },
    { jwin_text_proc,       6+10,  101+20,   24,    36,   0,        0,       0,       0,          0,             0, (void *) "D4:", NULL, NULL },
    { jwin_text_proc,       6+10,  119+20,   24,    36,   0,        0,       0,       0,          0,             0, (void *) "D5:", NULL, NULL },
    { jwin_text_proc,       6+10,  137+20,   24,    36,   0,        0,       0,       0,          0,             0, (void *) "D6:", NULL, NULL },
    { jwin_text_proc,       6+10,  155+20,   24,    12,   0,        0,       0,       0,          0,             0, (void *) "D7:", NULL, NULL },
   
    
    /* If I make it possible to edit these, too, they'd be here. -Z
    {  jwin_edit_proc,         6+10,     10+29+20,    90,     16,    vc(12),                 vc(1),                   0,    0,          63,    0,  NULL,                                                           NULL,   NULL                 },
    {  jwin_edit_proc,         6+10,     10+47+20,    90,     16,    vc(12),                 vc(1),                   0,    0,          63,    0,  NULL,                                                           NULL,   NULL                 },
    {  jwin_edit_proc,         6+10,     10+65+20,    90,     16,    vc(12),                 vc(1),                   0,    0,          63,    0,  NULL,                                                           NULL,   NULL                 },
    {  jwin_edit_proc,         6+10,     10+83+20,    90,     16,    vc(12),                 vc(1),                   0,    0,          63,    0,  NULL,                                                           NULL,   NULL                 },
    {  jwin_edit_proc,         6+10,     10+101+20,    90,     16,    vc(12),                 vc(1),                   0,    0,          63,    0,  NULL,                                                           NULL,   NULL                 },
    {  jwin_edit_proc,         6+10,     10+119+20,    90,     16,    vc(12),                 vc(1),                   0,    0,          63,    0,  NULL,                                                           NULL,   NULL                 },
    {  jwin_edit_proc,         6+10,     10+137+20,    90,     16,    vc(12),                 vc(1),                   0,    0,          63,    0,  NULL,                                                           NULL,   NULL                 },
    {  jwin_edit_proc,         6+10,     10+155+20,    90,     16,    vc(12),                 vc(1),                   0,    0,          63,    0,  NULL,                                                           NULL,   NULL                 },
    */
    { jwin_edit_proc,      (90-24)+34+10,   10+29+20,   72-16,    16,   vc(12),   vc(1),   0,       0,          12,             0,       NULL, NULL, NULL },
    { jwin_edit_proc,      (90-24)+34+10,   10+47+20,   72-16,    16,   vc(12),   vc(1),   0,       0,          12,             0,       NULL, NULL, NULL },
    { jwin_edit_proc,      (90-24)+34+10,   10+65+20,   72-16,    16,   vc(12),   vc(1),   0,       0,          12,             0,       NULL, NULL, NULL },
    { jwin_edit_proc,      (90-24)+34+10,   10+83+20,   72-16,    16,   vc(12),   vc(1),   0,       0,          12,             0,       NULL, NULL, NULL },
    { jwin_edit_proc,      (90-24)+34+10,   10+101+20,   72-16,    16,   vc(12),   vc(1),   0,       0,          12,             0,       NULL, NULL, NULL },
    { jwin_edit_proc,      (90-24)+34+10,  10+119+20,   72-16,    16,   vc(12),   vc(1),   0,       0,          12,             0,       NULL, NULL, NULL },
    { jwin_edit_proc,      (90-24)+34+10,  10+137+20,   72-16,    16,   vc(12),   vc(1),   0,       0,          12,             0,       NULL, NULL, NULL },
    { jwin_edit_proc,      (90-24)+34+10,  10+155+20,   72-16,    16,   vc(12),   vc(1),   0,       0,          12,             0,       NULL, NULL, NULL },
    
    { jwin_text_proc,           112+10+20+34+1-4,  10+29+12+7,     35,      8,    vc(14),                 vc(1),                   0,       0,           0,    0, (void *) "Action Script:",                      NULL,   NULL                  },
    { jwin_droplist_proc,       112+10+20+34-4,  10+29+20+7,     140,      16, jwin_pal[jcTEXTFG],  jwin_pal[jcTEXTBG],           0,       0,           1,    0, (void *) &lweaponscript_list,                   NULL,   NULL 				   },
    