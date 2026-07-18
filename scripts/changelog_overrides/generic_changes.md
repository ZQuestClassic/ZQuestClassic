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
subject 20e5b2415ba90cb1f172e408d1703b8ffa1ee800 fix: eweapon Rocks no longer hardcoded to die on solids
subject 383b5d34a6d33963e0a0caef3f6ff2c1c49d920b refactor(zq): Upgrade sprite data list dialog
subject c4a6d8b720e294688fa688828f276e2caf5c1584 refactor(zq): Clean up `Quest->Audio->MIDIs` lister
subject ec60344a6bbe40e152d15bd316d35c0426da1315 fix(zscript): Make `npcdata->Flags` a bool array, remove `->Flags2`
subject 35c95f6cc8bdaf374cfef3bf06a4370e573a9cae refactor(zq)!: Update door selector to zcgui
subject e5cf4d4dab7aa57e7d0d91e7eecfa03ae604349d feat(zq): Add descriptive labels and dropdowns to walker type attributes in enemy editor
subject d253cbb843b7652810c69b3e9d7044c1c136fef2 refactor(zscript): optimize sections of zasm that are run only for their side-effects

drop 165b42156443f963f41ce0f5a68638010ff7c137 feat: setting for overlay subscreens to hide during message strings

squash 2571479f44d6a99c06e097e7c6a34378d4e14e28 refactor(launcher): remove title option
squash 17e56f1410dc0bd04183ba13eda55e4892979b7f fix(zc): put title.mid back in zelda.dat, since it is available to quests
squash d6878d6d2a7cffa96ba3090ac29c7b2f5bf758d7 refactor(zc): remove title option from menu
squash dcd117a1a9d4c9aefd653f5a56f090ad231d394e misc(zc): remove unused title/logo assets from classic_zelda.dat
squash bcfd8bbf59023ddc0ca8db77b260dadd6e8504b3 refactor(zq): remove defunct -notitle switch from quest packager
pick e4640ab770d6aae79df2a2748646edd8d6fb63ac refactor(zc): Remove the original Zelda title screen

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
# ! accidentally duplicated this message
drop 774573f246c6ab49609f653198d7e3b11597fe79 feat(zq): implicitly add `<qst dir>/scripts` to include paths

subject 3c86a76b8d93c250bcf636ce19c3206be79b94c9 fix(zscript): 'delete' within a destructor being buggy
subject d188ece0e85762c8238e4081951da877a0b458e9 feat: option to disable transparent darkrooms stacking multiple transparent layers

drop 287e74d84200c3abdd7f1750207a28200531a09d fix(zq): reenable floating selection for dense forest

reword 88d94ba3c1917b36a3801e2d93a00f5f80554ec4 feat(zq): -smart-assign CLI switch to compile + assign
Example usage: ./zeditor.exe -smart-assign your_quest.qst
=end

subject 86b88dff9c410348fc888a312d68745bda857b2e feat(zc): Show bottom 8 pixels by default for most quests made prior to 2.55.9
subject cf2769fe95ae01ba07d2a028c803e284fdecc6a8 feat(zscript): Arrays now are separately typed from non-arrays

drop 99fedb984d3bbb1b2e49bdd968e873712e78285d fix(zq)<: Update cambria tileset

# ! trigger examples

drop 4a50b8a0167d41f5425e6a8cf233d3a99b9cba4e misc: update `quests/examples/trigger_example.qst`
drop 73f5b12e58e684cc1c066729639f981506524210 misc: update `trigger_example.qst`
pick 2e886e3e1473be779843e5055d3a723c7d7ba88b feat(zq): Add `quests/examples/trigger_example.qst`

# ! don't hide
reword 1b4fd9b5077b6970da14f11fe594f9edfa7e8674 fix(zc): armos/graves not activating properly from Large Hitbox player
=end

drop 8ec6c7c39922b3f8f348d893b697783776ff9f4d fix(zscript): prevent var declaration having wrong flags in ASTDataDecl
drop 39098a4a1dca13bf5d7dd467d72016a50975c73d fix(zc)!: handle older replays in replay_get_zc_version_created
drop 236e4bc8396602c1101a7cb5c0b0bc1262b0122f fix(zq): screen enemy list dialog constantly resetting focus
drop d82e808320a33d6daa957231866383c23122118b fix(zq): item advanced paste not pasting item name properly
drop ab9ad2fc84b5ba56b2cac094ef962fba063057ad fix: prevent rare crash when destroying null midi
drop 209b276703b4418fef1f33f2c01142e392497aeb fix(zq): "Follow Warp" not respecting dmap xoffset
drop 7ff9ca9c9621ba566062020ae358bb4d2763dd70 fix(zscript): improve unsafe code in STARTDESTRUCTOR/ZCLASS_MARK_TYPE commands
drop 8fdcbfe65c1c911de2d445d13fb5d2dac8b37053 fix(zq): typo in epilepsy qr helptext
drop 842d2a0bcb0eafde8550c6a36ce658a53657043b fix(zq): layer dialog not marking quest as "needing to be saved"
drop 44df6116058a4acfd3910e9bdcb208c618b1add0 fix: save menus not stopping sfx before opening (ex. ocean sounds)
drop f7a7e2a825ae5927a7ed3cb8b64c945da49b77de feat(web): add zasm output to zscript playground
drop 3ea93b743739fb402ff4c3524d61d74e74bd596d misc(zscript): re-number zasm comment numberings
drop 0faa3c2b6d1e1aa3aed7a9012edc7a2f5143d3fa feat(zc): remove dead ZASM

# ! obsoluted
drop 4be7097b035eff3e44a75370f25a9587f2d5b28b refactor(zscript): upgrade to jitted script without stalling game

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
drop 8a3f7b5de7d814cdd8598fb3d8a9e72682f5e153 fix(zscript): internal label error in some cases
drop dc424156c65998ec48c39d83768815c01d90331e docs(zscript): clean up docs for some global functions
drop fafa5cba970453584766fcf419ef03fdf7e87cd0 fix: prevent rare crash related to loading fake weapon script

drop 14267e6b6fd29a990b66438ed35ccb9e79d9ce74 feat: subscreen minimap compass can point to boss instead of McGuffin
drop 9a75c5452b9837bf6ac84a0441728e9e513ff422 build: upgrade sentry to 0.9.0

drop d00a94dfb186dd0fb49d9cf68d6776cfca482ed3 build: update websocketpp to newer develop branch
drop ae375c53032c4ce491c70330d44c83ee138e3f1e fix(zscript): websocket pool being destroyed on 'cont_game()'
drop 764e539e0fb340d2e231448ee74ae5f9b5c852d1 refactor(zscript): document, clean up, and sanity check websocket stuff
drop ce2f07d9c5a535ca67fc935c114494f0f4f3734d refactor(zscript): document, clean up, and sanity check websocket stuff
drop 83df3a67bcf48288e64b429dcc2be5627301e01f fix(zscript): websocket double-deleting their arrays, doing extra alloc/dealloc work
drop 32996be02bd5d7eee2de4b86de266d00487b1692 fix(zscript): crash in UserDataContainer, broken `websocket->Free()`
drop f98c49c6966990aa76cb43a0f993634caa17fe19 fix(zc): websockets reading wrong pointer
drop a2801b774c7ec896b83cd0a623ef4aafca9952df build: fix websocket cmake project from breaking CMAKE_CONFIGURATION_TYPES
drop 26c78ebec25fcaab7db36521f0e4f3a894e9a8a0 fix(zscript): '@AlwaysRunEndpoint("off")' erroring despite being valid
drop a12dbe335c222b47601737b9e9b06051c35fa2f1 docs(zscript): update webdocs
drop f17fa5f6a1b5eae327e003b0e44bef0aac767c83 docs(zscript): use .. version*:: admonitions for '@version' docs
drop 33effdbf0d5b560b1758767ea27d246f6cdb4475 docs(zscript): add examples using `Screen->DrawOrigin`
drop 52d0aa35ad1087803e2250ed80d7ebf3a88ebdf5 misc(zc): temporarily disable compare zasm opt as I improve in a branch

drop ab128735140decc4e634aef7fd0732c9fd837bbc fix(zscript): prevent compiler crash when parsing long decimals
drop f318313547d88c43bf347afb31d39f6e4c76a709 fix(zq): visual bug with Lister dialog preview size
drop 5abdbac9d734fea41b50df77a1894484c0ccda3b fix(zq): Map Styles dialog not counting as a "change" that needs saving
drop a6d3ae9701173dd3b046d381832735539d54939e fix(zc): Heart Container / Magic Container cheats using outdated values
drop 9d50f2f92f34391f4cc4f1bd517a8205beaf2fa1 fix: prevent out-of-bounds write from invalid save menu count
drop 9ea87637b022559078de9b42e0a4f1222e4ebc05 fix(zc): harden replay assert against source dimension changes

# ! vs code extension

drop 5526c1ddc2511e64f9e97452606af15fd1f80986 misc(vscode): publish 1.0.8
drop c58661f071c6b6e0f09ceb4dc13cc667371401f4 misc(vscode): publish 1.0.9
drop 790dc584ff36aac6390de58d79410ae2bc85c307 misc(vscode): publish 1.0.10
drop 23981d26c925e8ca6affd213e60fec46754a39c6 misc(vscode): publish 1.0.11
drop 87e68c33c1b34fe8970415c8be595edea9b78545 misc(vscode): publish 1.0.12

# ! reverted
drop 636cd693ebcc1cbec50ced5af8ece8be31f04f64 fix(zc): only grab one held item at a time

reword c4ff8de1a0e9aa5f2c9aeb7ebfef556d8ffc20d2 feat: Add many new SCCs
- CollectItem
- Delay
- ForceDelay
- KillHero
- Counter
- MaxCounter
- GoIfAnyScreenState
- GoIfGlobalState
- GoIfLevelItem
- GoIfLevelState
- GoIfScreenState
- GoIfSecrets
- GoIfScreenExState
- GoIfAnyScreenExState
- SetGlobalState
- SetLevelItem
- SetLevelState
- SetScreenExState
- SetAnyScreenExState

See https://docs.zquestclassic.com/tutorials/message_strings for details.
=end

# ! drop CI stuff

drop 1a8cbf99daded2c578e1163a853831e0e92bbb9b ci: reset cache for web builds
drop 16ce632d59cc0e38f7ee2e6a397ddbb95a56a2c1 ci: for web tests, build for RelWithDebInfo
drop c0e0151c499e52c41af50a84f2c517971a1ddc4b ci: include coverage for all apps, not just zplayer
drop 75da202d66b1f3d3e1ad37ca44a6374756ad0a6e ci: bump build cache so 2.55 branch doesn't conflict
drop 15b7ff4aa5da3829107662191d7444126e4040e8 ci: get database update cron working again
drop ec499225b80237aeeaa055ab0043e5229577c69b ci: fix type in download-artifact v4 action
drop e272d0914fd05db5944f2477cff73d66c33f8804 ci: fix another typo in download-artifact v4 action
drop bfd9daa06a00e56f506e14ed67087d3ffc9f90c1 ci: set test results folder names for jit/non-jit replay runs
drop bbb53680f956136f2c1417b4b1d5aff042ccd58b ci: fix replay report generation for forks
drop 12b1fa6541c20cea165e22259cb10d142721013d ci: upgrade to python 3.12.8

# ! 3.0 changelog audit: drop fixes for behavior/features that were themselves introduced after 2.55.0

# ! misc internal stuff

drop ec0a84ab43e2d231f8065593309e49a8b6dcf675 refactor(zc): add null jit backend
drop 847ab88e9674e3393291add66bf205b93def4b16 refactor(zc): move debug zasm writing to new -extract-zasm command
drop 67be7ecd1115c0415e90345638d0cda768c03f12 ci: cover wasm JIT frozen-generic scripts and mid-run adoption on web  

# ! fixes to the ZASM optimizer (new in this range)
drop f55cf241f42a32ee96437be006fcebfa4175052c fix(zc): re-optimize when jit reuses scripts from last load
drop d482c135b98ba01ed043d2109f1a2af9384d6bb7 fix(zc): invalid optimization on broken SDDDD register
drop cca0aaf0d657b2409a9183771f017254514ee2be fix(zc): disable propagate_values and dead_code passes for now
drop a3d768e8f335093bd7a025761fd8c3b0edb63475 fix(zc): inline function optimization handles PEEK correctly
drop 47ca63d081ae4509f45d80497fc5292a6cd058d5 fix(zc): various fixes to optimizer that broke crucible quest
drop ffd3fe1fec0ad60cecf5e33076801826665dd251 fix(zc): inline function optimization handles PUSHARGSR correctly
drop 2ed2cd56dc507f6a2186071547a3fdc910b670b2 fix(zc): bug in dead code optimizer when register is both read and written to
drop 0c8cc86242f8d6dce7aef1bec240f39be6d9a918 fix(zc): inline function optimization handles LOAD correctly
drop 694d589f6feb31bd29ce4269b38ed7e0ea01b676 fix(zscript): use correct config section for optimize_zasm

# ! fixes to structured-ZASM utilities (new in this range, added with the wasm JIT work)
drop 68259bbf46ba0e56f3da4b31aa9c7bfc16f05aec fix(zc): improve function breakdown in structured zasm
drop 69bbf846a21ba1cae2fc14e65f937541ca16a67c fix(zc): handle first command being function call for structured zasm
drop 1649a62e1251a3b616e166f3bdff40b0c96cb7bc fix(zc): handle recursive function calls in ZASM cfg creation
drop 08e17d01de2d6632f7824788faf7e3137a890e5c fix(zc): detect dtors as separate functions in structured zasm
drop c45c5f5e7db4ad6896d429e6ee4694b3d779c9e4 fix(zscript): consider PEEK when finding function calls for structured zasm
drop 6c4d774c56b8630d4afa36bfb17eac256140313c fix(zc): check for null when printing ZASM command string arg

# ! fixes to compiler passes/optimizations that are new in this range
drop 8d2ddc6f90aefb09846da5c15c6447aa08fc4ef7 fix(zscript): missed handling arrays for unused var trimming
drop d8123f886f6c689316006a14ab6898be2b0edd78 fix(zscript): invalid compiler output from removing unused variables
drop 978ab1b634fa6026f471c2afc60e31e3a09ed5a2 fix(zscript): parser bad optimization edge-case
drop b000fc9e2f02dded946b8fa1dd81477b4a8d0797 fix(zscript): bad optimization causing global vars to sometimes read incorrectly
drop 36be5897a91ea8307bb6ccf6aeac580c3173c6ae fix(zscript): vargs functions wrongly pushing extra values
drop ccf9959570c054ff916ff6b15daf6ac44c18e913 fix(zscript): errors related to breaks in infinite loops
drop 8721621154b44903003c90b1ea1bf1a4f718d1e0 fix(zscript): issue with global variables sometimes not initializing properly
drop bfef7fd4456308e0c301bf0b7443546aaf0a18db fix(zscript): label error bug with 'while(true)' and 'break;'
drop b3fb9bc30868cae9c3e518d738c8c478df1d1a3f fix(zscript): case-range backwards check
drop b93b1e22495390c241d0993725878c45029d7a3e fix(zscript): parser failing to properly initialize some variables
drop 5f8f9d1246030f4212e1ec33ccfa5f651cc90412 fix(zscript): x64 jit compiles STACKWRITEAT correctly

# ! fixes to editor metadata generation (new in this range, powers the VS Code extension)
drop 54cb5c3ac694eb2e3b7a834f0fa9a09fc5a829a4 fix(zscript): crash from metadata of var inside if statement
drop 235ad808bfd12dcb49fa8e59c708e6aaf5a39a71 fix(zscript): prevent crash when processing for-loop metadata
drop 3d5912f157235ee210658281b5876d9f2b3bb8cb fix(zscript): continue generating editor metadata after (most) compile errors
drop 8defe2955b2fbe690ce56ade344e32f81981fa82 fix(zscript): generate metadata for code even if optimized away
drop 68a5f70c82dd870ee7813e9e971f182751040803 fix(zscript): 'loop()'s not having location metadata associated with their identifier

# ! fixes to ZScript language features that are new in this range (GC/objects, bindings, templates, range loops, websockets)
drop 4f80edc073629fa60d7c557fb7f352bfd5e86ae3 fix(zscript): `Game->GameOverScreen[]`/`Gam->GameOverStrings[]` not compiling in 3.0
drop 12e3b9a92a4983f051d85772a0ab2216aa667a4a fix(zscript): socket arrays sometimes going invalid?
drop 08756b8cb16576d985678188a9eeb782d7c31c4d fix(zscript): allow simple types to match a template array type, for now
drop 7feb9afdda39fe7e8ef6a7713eade772c4171ccc fix(zscript): object fields not being available in dtor
drop b00f59aadb09ab987829d549b71c5bf9e032d8ee fix(zscript): compiler crashing on for-each loops
drop 975cf9c6048c42e24e14780f2c0d79e6c1d8e683 fix(zscript): some bindings ZASM using wrong literal values
drop 21336156492fd46a2222476f379e45935eac45d7 fix(zscript): code gen for range loop using wrong op
drop dc6c35037ef26c418299a7159064891a1108bc07 fix(zscript): stack offset issue when 'return'ing inside a 'loop()'
drop f93172d335fb280606c8586e73bdf648e43eac44 fix(zscript): handle non-global objects correctly when reloading game
drop 4bd0e5412578d4d040cb2659f9c96f951098b3f0 fix(zscript): not correctly removing stale pointers on reload
drop 2413e5ed842c64f4b775eb27c5de1b85f20afb1c fix(zscript): code blocks sometimes not removing object references
drop 4a89897b67196d9d39e4cfc2d8fb67a7db225be7 fix(zscript): deprecation warnings for functions not giving 'INFO:' text
drop 4ce3a218621e6d125b1c5251ec3ddc5c3e994b99 fix(zscript): 'hero' and 'link' script types not compiling
drop 7787176771150d77dcc25ec1e973cb12ae66e17f fix(zscript): internal bitmaps from `Game->LoadBitmapID()` now work with Blit

# ! fixes to the wasm JIT / web features that are new in this range
drop eb3dc8a7a0d9e65bf9167ea15f10f3a71e0af496 fix(web): handle reaching end of script in wasm jit
drop 35fc59b6b3a8eb7e95fcebd1b1bb9792cf4890d8 fix(web): handle CMP_BOOL in jit wasm
drop 5f74b174a61d23f18a5d72091e32713097e16800 fix(web): prevent WASM JIT hanging on some older quests
drop 2e6d369c748dd1d178c9103724bac4ee2fbc8378 fix(web): handle a comparison feeding multiple commands
drop 8699ead6e70ed939be3e9a85046160cb1ef218d1 fix(web): finish a non-yielding script that returns
drop d3fe23be98b9c1518482440d8eb1fa8d289e7bb2 fix(web): evaluate a pushed operand before decrementing sp
drop 27096a5825fa1affd099ea102652c11ec61aa710 fix(web): stop a JIT crash when one script runs another mid-frame
drop 3a007147c40d37bebfe4b06bfcfcca02247480a6 fix(web): stop a JIT crash when one script runs another mid-frame
drop 43d53f4e83641e81358130f96ff782620c402a56 fix(web): scripts started during another script no longer restart
drop 431730fd3d07a6311296ef2f8b17ea4db7d381f7 fix(web): don't crash when loading a second quest in the same session
drop cc0518ae6d05df6f769f622afd4e8ea7acbec49d fix(web): disable broken parallel processing for zasm opt
drop bb92a873970441b064023c28935ae7ba01509fe9 fix(web): stop cheat overlays from crashing

# ! fixes to player features that are new in this range (titlescreen, regions, 8-bit palette, new widgets/flags)
drop a5a9bc81d1b232b14ddf223ae0fb30626fb7e92f fix: zscript docs were not being generated for release package
drop 8f50a32f5a85b3717494065106ff2e0a1c074e94 fix: 'Screen->Lit' not compiling; now properly returns (and sets) screen's lit state
drop 1d08708c9cc262ce57f17ceb92afd953d919893a fix: initial path not being set for file dialog
drop 94f39f26e2246469b9549cc751f056f681335706 fix(zc): skip title screen if specific save slot is given
drop c3f7ec46fd72b3cf3f712528cd2ec0df231cd25e fix(zc): actually stop mp3/ogg, and explicitly stop title music
drop 3967f0ebfc80c010c5df8b2bd11368ba672e97a1 fix(zc): apply switch state when checking bordering screens
drop 226cc65c8ff6017608b55acea62afb212ffab9d4 fix(zc): Subscreen 'Sys Colors' displaying entirely wrongly
drop f564449997ee3030af96116ba2129a73476e293a fix(zc): Stunned hero no longer stops dead in his tracks on ice
drop 35095ceae9f2792274a0a2def614137d4b79b6fb fix(zc): Bottle flag 'Cure Shield Jinx' not working when used manually
drop caa206e9b8a2a9024d54dac0a20e6c5e10549d21 fix: the counter percentage bar widget was drawing completely wrong rectangles
drop b300fcd8212cafa82a3e4215e578fbfd0689be77 fix(zc): issues with sideview "gravity falling" pushblocks
drop f3e92ba962f4e710b38fe233b82dc919bdc9d56e fix(zc): 'large' armos not detecting proper positions to change statues of
drop e8c221c605350ba94fdfcdae6c8c6553992253ad fix(zc): strings parsing after being told to close

# ! fixes to editor features that are new in this range (zoom out, move-code upgrade, notes, new dialogs)
drop c599dfc30d948228a5915e7c3b33f08b9489f90b fix(zq): test init data not being applied correctly
drop 5c338087d82dde4f93a1a96658b63723a951fb0e fix(zq): 'Notes' and 'Browse Notes' not having menu buttons
drop 80af019ef4835ae40dce4db8f3f1ecebe89f1f14 fix(zq): crash on moving combos in quests with many maps/screens
drop 4efe6b07dff7f9a504fd20f877bd92abf66ae758 fix(zq): Improve efficiency of combo moving + fix crash + fix 'undo' combo move
drop 7b26369e090ecb09a7beee13542a06d6a6f4f570 fix(zq): Improve efficiency of tile moving + fix 'undo' tile move
drop 3c72e37128477a3cfae9e533f81c6966373ac2dc fix(zq): memory issue when moving/inserting tiles/combos
drop 143e6836c638f96ef791fa73c8f35c2b2694dc98 fix(zq): prevent crash when assigning global scripts
drop aa28815b4e1006a989f5d00c2b089a5b80ea6db2 fix(zq): prevent another crash when assigning global init script
drop dc8d75038c0d325d53b23f14b7fb9665f87fcbb1 fix(zq): combo editor triggers -> Level Palette now has hex/decimal swapping button
drop 83d31d5a54d6dd69ed0c772286e143188f468696 fix(zq): aliases with layers set not placing correctly when zoomed out
drop c8eeddfdf496f7c2cded75f20155d9accaf08868 fix(zq): Subscreen TileBlock Wizard not saving Width/Height values properly
drop 0eb511287d5b75da7ffc0d87792182c5212c2b79 fix(zq): Naming of tile variables in enemy lister info

# ! fixes to VS Code extension features that are new in this range
drop b5b8ef720c981532c5524f7827778d815b609f5c fix(vscode): 'AlwaysRunEndpoint' annotation missing syntax highlighting
drop 179d55946cf29d5917def708e35095be36922e3d fix(vscode): invalid uri on windows for links in hover tooltip
drop 2d876df1a2fa94d1cf26d148d3f874e3ade7e89e fix(vscode): correctly highlight half-open ranges

# ! superseded by 6c30566233 (kept), which reworked bridge coverage within this range
drop 2fb6d62fd1ed20bd4c1fa4d4a669d72311c7f265 fix: more bridge problems

# ! 3.0 changelog audit: typos and unclear language

subject efe6125234813ea0759a43b5fb22c75da59753b6 fix: Modulo operator on decimal values being entirely wrong
subject e92a4899f8f5ff8d93639f9a31c4c70de3638ae0 feat: Allow 'walk on top' for all combos, not just Switch Block combos
subject 284fc7567d8846ab3867235de14995dd5906a599 refactor(zc): use translucency table cache for all fade/lighting
subject ded10db593addd24f95fd8d8b0b3696fe52da658 feat(zscript)<: range loops have '@NoOverflow()' annotation, docs

reword d460b968dd4833de21fb8d627ab85286a5d5c5c6 feat: FFC Flag for solidity based on the combo being solid or not
The whole ffc will be either solid or not, based on if the combo has at least some solidity.
Useful for FFCs changing via triggers.
=end

reword 7aabd9078292894758a7ddbad582ff51d134271b refactor(zscript): simplify ltou, utol and convcase
These functions returned false if the string given is empty. That's not useful, so now they no longer return anything.
=end

reword a959db9840b491d89eb8c1f4e029e622094e3ac6 feat: Summoner Improvements
Enemies that Summon:

- now have consistent configurations for summon count between Walking and Wizzrobe types
- now allow configuring max enemy count
- now count specifically enemies that they have summoned when checking against max enemy count
- have a new flag to kill summoned enemies when they die (targeted ringleader effect)

Scripts now have '->SummonMinion()' and '->SummonMinionFromLayer()' to make an enemy summon other enemies. Scripts also have '->ParentSummoner' to set which enemy is considered the summoner of other enemies.
=end

reword 960e00c524fc13dfd40b792e3a878671f16b5e52 feat: Gravity Boots
Coming in 3 varieties, these boots alter the Player's gravity and terminal velocity!

- 'Gravity Boots' just change them passively (can optionally be sideview-only)
- 'Gravity Up Boots' change it when holding 'Up' in sideview
- 'Gravity Down Boots' change it when holding 'Down' in sideview
=end

reword 88ca703ab061b564c996a26a4ac99e564b7fb858 feat: remove combo 'attribytes' and 'attrishorts', expand 'attributes'
- attribytes 0-7 are now attributes 8-15
- attrishorts 0-7 are now attributes 16-23
- attributes 0-3 are still 0-3, 4-7 are new

This will allow larger / negative / including-decimal-part values in some cases that would not allow them before.
=end

reword 80d0b60228a626548a1508868ecad4116cd71104 feat: `Partial Consume` combo trigger for counters
Allows the `Consume Amount` to occur even if you don't have enough of it. Useful with `...Is Percent` for ex. `Consume 100%` to empty a counter.
=end

reword c832de843fdddc821a653f798b99b9bdf14fd3bb feat: 'Chance' trigger condition
Set a combo to have an N in M chance of its trigger actually happening. Useful to actually have randomness in trigger setups!
=end

reword cd73a160707c7c441e8045b11ce5f698b6c88ff3 feat(zc): "Temp Ignore Solids" weapon flag
Causes a weapon to ignore solids for the 'Break on Solid' / 'Stop on Solid' flags. Un-sets itself as soon as the weapon is NOT touching a solid. Using this flag on a solid shooter combo will prevent the shooter itself from killing its shots.
=end

reword 3643d5c2619e08b457c0540297a1a2333b06373c feat: per-position states
This allows specific combo positions to remember changes between screen transitions. Each combo position and each ffc gets 8 states (0-7). The memory usage of this feature may add up if used excessively.
=end

reword 99a548a5248fa316803c81fc351adb7a2f55d894 fix(zscript): produce correct codegen for objects with default constructors
Creating a custom object inside another's constructor generated incorrect code when the second class has a default (or empty) constructor.

For example, this code would error and not set the `b` variable on `f`.

```
class foo
{
    bar b;
    foo()
    {
        b = new bar();
    }
}

class bar
{
    bar()
    {
    }
}

foo f = new foo();
```
=end

reword 6c305662335b84bc5f82b27c00aea9cb1e59308e fix(zc): various fixes for Bridge combos
Bridges previously were not covering some things they should - this fixes many of them. Notably:

- Warp, Save, Step, Reset Room combos
- Win Game, Fairy Ring, Weapon-based placed/inherent flags
- Bridges above layer 2 for most things

Also made the code for handling bridge combos MUCH cleaner.
=end

reword 56e9081285590f7494d1c37787b71ef515509dee feat: multiple triggers per combo
- Each combo can have anywhere between 0-255 combo triggers. Only the used triggers take up space (in memory or qst).
- The triggers are checked in order, so if the same combo has multiple triggers with the same cause, they will run in the specified order.
- If in such a case, the combo is changed while there are still triggers remaining, the extra triggers will NOT be executed - the execution halts when the combo is altered.
- Editable string Label for each trigger, used in the display name in the combo editor list
- Script access. The old access should all still work, accessing the combo's *first trigger*. Access for new triggers uses the new `combotrigger` datatype.
  - `Screen->TriggerCombo()` can specify trigger index
- The 'Only Gen Triggers' flag is now part of combos instead of triggers, so has separate access (`combodata->OnlyGenTriggers`)
- `combodata->GetTrigger()` to get a specific trigger by its label (invalid names safely return a null value)
=end
