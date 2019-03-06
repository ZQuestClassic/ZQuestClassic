const char *dmapscriptdroplist(int index, int *list_size)
{
    if(index<0)
    {
        *list_size = bidmaps_cnt;
        return NULL;
    }
    
    return bidmaps[index].first.c_str();
}


//droplist like the dialog proc, naming scheme for this stuff is awful...
static ListData dmapscript_list(dmapscriptdroplist, &pfont);
