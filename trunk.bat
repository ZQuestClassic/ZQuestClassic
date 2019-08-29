call backup %1
echo y | copy %1\*.* _zcrepo\trunk
echo y | copy %1\parser\*.* _zcrepo\trunk\parser

