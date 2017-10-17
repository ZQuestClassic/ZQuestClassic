call backup %1
echo y | copy %1\*.* _zcrepo\trunk
echo y | copy %1\parser\*.* _zcrepo\trunk\parser
echo y | copy *.dat _zcrepo\trunk\output\common
echo y | copy 1st.qst _zcrepo\trunk\output\common
echo y | copy 2nd.qst _zcrepo\trunk\output\common
echo y | copy 3rd.qst _zcrepo\trunk\output\common
echo y | copy proposed1st.qst _zcrepo\trunk\output\common
echo y | copy proposed2nd.qst _zcrepo\trunk\output\common
echo y | copy proposed3rd.qst _zcrepo\trunk\output\common
echo y | copy zquest.txt _zcrepo\trunk\output\common
