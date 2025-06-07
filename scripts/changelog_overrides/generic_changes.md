subject ae823474a24261e92266df611c62a5f6007c33d3 fix(zc): waitdraw not working for ffcs over 32
subject c8e2c861edc73de3ff851d3cbc9bc8d07037d007 fix(zc): npc scripts not running on screen load
subject 935757d42efb0f999d7a5a1b0d2e893cac962614 fix(zc): enemies not running scripts on first frame with scriptloadenemies
subject 95e3288ec29c716b8645a2e01da311a94fded6b5 fix(zc): Test Mode issues when quitting during opening wipe
subject 9bf02571c6c09c182561bd565fc13fe701d4ad04 misc: ./zelda -load-and-quit `<qstpath>`
subject 5641cf8c1a9e76ed8fca68e7189f71c5efd20e1e fix(zc): new respawn points with ladder
subject 9762141dbe1bd7461b5dee03a627f290c6ed322a fix(zc): various newer hero movement
subject 0da042ba3b238bba1b468c753f24cce71cefc2a4 fix(zscript): Negative Array rule issues
subject dabe7c091b89fa2e09217dfb63c652184b78f65e feat(vscode): create VS Code extension
subject 6ecf0775afeac108bb4f806b76f9c1835c0a98d1 feat(vscode): update vscode extension keywords
subject d82a0d8458b3029283385db04f1d935fd0dbce48 misc: add 'std_zh' scope to changelog generation
subject f7228536ad55c2fb4bb20d00bba8b8c06d7e67c2 feat(std_zh): add 'GetLevelSwitchState()'/'SetLevelSwitchState()' helper functions
drop 165b42156443f963f41ce0f5a68638010ff7c137 feat: setting for overlay subscreens to hide during message strings

squash 2571479f44d6a99c06e097e7c6a34378d4e14e28 refactor(launcher): remove title option
squash 17e56f1410dc0bd04183ba13eda55e4892979b7f fix(zc): put title.mid back in zelda.dat, since it is available to quests
squash d6878d6d2a7cffa96ba3090ac29c7b2f5bf758d7 refactor(zc): remove title option from menu
squash dcd117a1a9d4c9aefd653f5a56f090ad231d394e misc(zc): remove unused title/logo assets from classic_zelda.dat
squash bcfd8bbf59023ddc0ca8db77b260dadd6e8504b3 refactor(zq): remove defunct -notitle switch from quest packager
pick e4640ab770d6aae79df2a2748646edd8d6fb63ac refactor(zc): remove title screen

drop 93953beea071045757d2d7b69b053149b3e2fc21 chore: fix last commit
drop 3d944ba6532d7fb96ed6eb6e78463c71add6775e build: fix windows build

squash ad58eb0ab7b2aec8689bfd29af3cd5f3bc52513f feat(launcher): re-open launcher after update
squash e11f6058b607fca5fcf6e951d3305537c95009fe test: re-enable updater test
pick 5b3162034226d0a7d0e00679df0080eb0fd5d16e feat: add software updater

drop 6e0ff07c5897ec6900e126d6b4bcf928a8fd9b13 chore: fix previous commit
drop 9f656d8abb1d08b276448a92dbbe605a93e9fd6a build: fix build for msvc

squash 593beae6594ff734d535b3664dd867b06bbb6305 test: move zquest save classic_1st.qst test to python
pick 7c8a5625c582a12b7d779cf6173f670543d25214 test: move zquest save classic_1st.qst test to python

drop 2769fc99965e764f2b2f809ea935bb15d5db4213 build: fix windows compile

subject 3c86a76b8d93c250bcf636ce19c3206be79b94c9 fix(zscript): 'delete' within a destructor being buggy
subject d188ece0e85762c8238e4081951da877a0b458e9 feat: option to disable transparent darkrooms stacking multiple transparent layers

drop 287e74d84200c3abdd7f1750207a28200531a09d fix(zq): reenable floating selection for dense forest

reword 88d94ba3c1917b36a3801e2d93a00f5f80554ec4 feat(zq): -smart-assign CLI switch to compile + assign
Example usage: ./zeditor.exe -smart-assign your_quest.qst
=end

reword 86b88dff9c410348fc888a312d68745bda857b2e feat(zc): show bottom 8 pixels by default for most quests made prior to 2.55.9

reword 62a494305b682a58c83104b47145245698f56d6f fix(zscript): `lweapon->isValid()` now accurate for lifted weapons
reword d7869fc0452d70aff8c797ae96bad62bccb31d92 fix(zc): lifted weapon not disappearing on death

# ! various fixes of bugs only introduced in 3.0 prerelease builds
drop 1782559d7e3bd63725c069e1411f453ca75fac9f fix(zc): Pound combos on layers changing the wrong layer's combo
drop 664bedf36632237ec9fb886509c49de3ade0e88c fix(zc): combo trigger conditionals for dark/not dark no longer inverted
drop a3cd7723bf674b9770d54253d2282e697a7a9579 fix(zc): Push (Right, Many) only pushing once
drop cb07f7ca5d6c40b2551ea954018d61de1c7d95a9 fix(zc): Icy Floor collision in sideview
drop 97e1435e81a9ef8ede7c614e876395a343fd9af9 fix(zq): add missing moveflags to enemy editor
drop 49fcf283f445ed496257f04218e25a7c4c3198d8 fix(zq): square misalignment in zoomed-out expanded mode
drop 02bc0f4ffba35b6bd81fbdc3e783f2574b543885 fix(zscript): a couple of small errors in recent optimizations
drop c043aaae32f611d5b74c05ebfcc9b2a1ca4beff5 fix(zq): next-screen-preview visual bugs re: zoom out
drop 2f845b504556032a00ac0c0a181a04a210f39b7f fix(zscript): fix many internal functions
drop 0903ecdc18ec6b1a7542e287218988928b497839 fix(zq): Screen Enemies Dialog not setting "Dirty" State for saving
drop ab5f763155939f2025ca8980084bb86404c2dd09 fix(zq): enemy editor spawn/death sprites swapped
drop 2d2c5de928d38512a80bd8b079ff3de3983017a1 fix(zq): combo alias cursor visual issues
drop aa467263cdf9d92eaaf7ab6e2ca4a8fee54ccf17 misc(zc): add -analyze-zasm-duplication for finding duplicated functions
drop 1152e5548ac155eba7ea08ff410d7b4e3b130319 fix(zc): Region 'Physical' mapping option not working outside of regions
drop 6be636e6d7c83d294de1c73019c29b97670bd537 fix(zq): description of 'Consume w/o trig' triggerflag was misleading
drop 41139dde14d5cfa964c555695f478feb320c9dd0 fix: use correct app id in zalleg_setup_allegro
drop 35505498d5a8e2c2668eb7a1d21715ed9af3900a fix(zq): add missing sideview ladder flags to top row of item cellar and passageway
drop 7f249ee2ec123fefdbc69b968d8f99fe71bf5c82 fix(zscript): fix broken array literals when declared as array pointers

reword 383b5d34a6d33963e0a0caef3f6ff2c1c49d920b refactor(zq): upgrade sprite data list dialog
reword c4a6d8b720e294688fa688828f276e2caf5c1584 refactor(zq): clean up `Quest->Audio->MIDIs` lister
reword 35c95f6cc8bdaf374cfef3bf06a4370e573a9cae refactor(zq)!: update door selector to zcgui

drop 14267e6b6fd29a990b66438ed35ccb9e79d9ce74 feat: subscreen minimap compass can point to boss instead of McGuffin