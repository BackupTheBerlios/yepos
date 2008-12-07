/*yepos the PalmOS dictionary: main() and other functions*/
#include<PalmOS.h>
#include"control_ids.h"
#include"show_battery.h"
#include"prefs.h"
#include"settings.h"
#include"../include/signs.h"
#include"enums.h"
#include"globals.h"
#include<SysZLib.h>
#include"zlib_error_alert.h"
#include"mem_ory.h"
#include"dict_header.h"
enum local_constants
{bits_per_byte=8,
 x0=0,y0=11,status_line_y=-2,articles_height=screen_height-y0-12,
 margin=2,
 log2_cache_length=4,cache_length=1<<log2_cache_length,
 cache_length_mask=cache_length-1,list_mode_inc_value=7
};
enum local_defines{facunde=0,zlib_irrobust=1};
static void
draw_chars(const char*s,int x,int y)
{if(facunde)WinDrawChars(s,StrLen(s),x,y);
 if(facunde>1)SysTaskDelay(50);
}
static unsigned long global_ticks;
static struct dict_header dh;
static unsigned zlib_buf_size;static char*zlib_buf;
const char*
get_db_comment(int*size){if(size)*size=dh.comment_size;return dh.comment;}
MemHandle idx_handles[2];
static const char*uncompressed;char*indices[2];
static unsigned current_article,current_content_record=1;
struct cache_item
{int db_idx;unsigned rec_num;struct mem_chunk chunk;
 const char*content;
};
static struct cache_item cache[cache_length];static int cache_head;
static int
init_cache(void)
{int i;struct cache_item*p;
 for(i=0,p=cache;i<cache_length;i++,p++)
 {p->chunk.d=invalid_chunk_descriptor;p->content=0;p->db_idx=-1;p->rec_num=0;}
 return 0;
}static void
close_cache(void)
{int i;struct cache_item*p;
 for(i=0,p=cache;i<cache_length;i++,p++)
  if(p->chunk.d>=0)free_chunk(p->chunk);
}static int
inflate_into_chunk(struct mem_chunk m)
{unsigned cont_size,decompressed;const char*cont;z_stream str;int err;
  unsigned orig_size;MemSet(&str,sizeof str,0);decompressed=0;
  orig_size=*((const unsigned*)*indices);cont=*indices+sizeof(orig_size);
  cont_size=MemPtrSize(*indices)-2;decompressed=0;
  str.next_in=(char*)cont;str.avail_in=cont_size;
  err=inflateInit(&str);
  if(err){zlib_error_alert(err,"inflateInit");return err;}
  while(decompressed<orig_size)
  {unsigned n;str.next_out=zlib_buf;str.avail_out=zlib_buf_size;
   err=inflate(&str,0);
   if(err&&err!=Z_STREAM_END){zlib_error_alert(err,"inflate");return err;}
   n=zlib_buf_size-str.avail_out;
   write_chunk(m,decompressed,zlib_buf,n);decompressed+=n;
  }while(err!=Z_STREAM_END);
  err=inflateEnd(&str);if(err)zlib_error_alert(err,"inflateEnd");return err;
}static const char*saved_uncompressed;
static int
find_cache_item(unsigned rec_num,int assign_uncompressed)
{int i=cache_head,d=get_current_db_idx(),idle_item=(cache_head+1)&cache_length_mask;
 if(assign_uncompressed)uncompressed=0;
 if(saved_uncompressed&&cache[idle_item].chunk.d>=0
    &&saved_uncompressed==cache[idle_item].content)
  idle_item=(idle_item+1)&cache_length_mask; 
 if(facunde)
 {char s[0x33];StrCopy(s,"cache find ");StrCat(s,"(");
  StrIToA(s+StrLen(s),d);StrCat(s,":");
  StrIToA(s+StrLen(s),rec_num);StrCat(s,")");
  draw_chars(s,0,0);
 }
 do
 {if(facunde)
  {char s[0x33];StrCopy(s,"cache ");StrIToA(s+StrLen(s),i);StrCat(s,":(");
   StrIToA(s+StrLen(s),cache[i].db_idx);StrCat(s,":");
   StrIToA(s+StrLen(s),cache[i].rec_num);StrCat(s,");");
   StrIToA(s+StrLen(s),cache[i].chunk.d);StrCat(s,"  ");
   draw_chars(s,0,0);
  }
  if(cache[i].chunk.d<0){idle_item=i;goto next_i;}
  if(d==cache[i].db_idx&&rec_num==cache[i].rec_num)
  {if(facunde)
   {char s[0x33];StrCopy(s,"found ");StrCat(s," ");
    StrIToA(s+StrLen(s),i);StrCat(s,":(");
    StrIToA(s+StrLen(s),d);StrCat(s,":");
    StrIToA(s+StrLen(s),rec_num);StrCat(s,")");
    StrIToH(s+StrLen(s),(UInt32)(cache[i].content));StrCat(s,"  ");
    draw_chars(s,0,0);
   }if(assign_uncompressed)uncompressed=cache[i].content;return i;
  }
  next_i:i=(i-1)&cache_length_mask;
 }while(i!=cache_head);
 if(facunde)
 {char s[0x33];StrCopy(s,"cache loop done ");StrIToA(s+StrLen(s),idle_item);StrCat(s,":(");
   StrIToA(s+StrLen(s),cache[idle_item].db_idx);StrCat(s,":");
   StrIToA(s+StrLen(s),cache[idle_item].rec_num);StrCat(s,");");
   StrIToA(s+StrLen(s),cache[idle_item].chunk.d);StrCat(s,"  ");
   draw_chars(s,0,0);
 }
 if(cache[idle_item].chunk.d>=0)
 {free_chunk(cache[idle_item].chunk);
  cache[idle_item].chunk.d=invalid_chunk_descriptor;
 }i=(idle_item+1)&cache_length_mask;
 while(i!=idle_item)
 {cache[idle_item].chunk=alloc_chunk(dh.record_size);
  if(cache[idle_item].chunk.d>=0){cache_head=idle_item;break;}
  while(i!=idle_item)
  {int i_prev=i;i=(i+1)&cache_length_mask;
   if(cache[i_prev].chunk.d>=0&&
      (!saved_uncompressed||saved_uncompressed!=cache[i_prev].content))
   {free_chunk(cache[i_prev].chunk);
    cache[i_prev].chunk.d=invalid_chunk_descriptor;break;
   }
  }
 }
 if(facunde)
 {char s[0x33];StrCopy(s,"idle alloc ");StrIToA(s+StrLen(s),idle_item);StrCat(s,":(");
   StrIToA(s+StrLen(s),d);StrCat(s,":");
   StrIToA(s+StrLen(s),rec_num);StrCat(s,");");
   StrIToA(s+StrLen(s),cache[idle_item].chunk.d);StrCat(s,"  ");
   draw_chars(s,0,0);
 }
 if(cache[idle_item].chunk.d>=0)
 {if(inflate_into_chunk(cache[idle_item].chunk))
  {free_chunk(cache[idle_item].chunk);
   cache[idle_item].chunk.d=invalid_chunk_descriptor;
  }else
  {const char*uc=lock_chunk(cache[idle_item].chunk);
   cache[idle_item].rec_num=rec_num;cache[idle_item].db_idx=d;
   cache[idle_item].content=uc;if(assign_uncompressed)uncompressed=uc;
   return idle_item;
  }
 }return-1;
}
static unsigned
idx_items_num(void){return*((unsigned*)(indices[1])+1);}
static const unsigned*
idx_items_array(void){return((unsigned*)(indices[1]))+2;}
static unsigned
first_record(int arity){return dh.ary_records[arity];}
static unsigned
articles_number(void){return*((unsigned*)uncompressed);}
static const char*
article_ptr(unsigned i)
{unsigned*title_idx=(unsigned*)(uncompressed);
 title_idx++;return uncompressed+title_idx[i];
}static void
unlock_handle(MemHandle*handle)
{if(*handle){MemHandleUnlock(*handle);*handle=0;}}
static void
free_indices(void)
{int i;
 for(i=0;i<sizeof(idx_handles)/sizeof(*idx_handles);i++)
  unlock_handle(idx_handles+i);
 uncompressed=0;current_article=0;current_content_record=1;
}static char*
alloc_zlib_buf(void)
{int i=10;char*p,*zp0,*zp1;unsigned zp0_size=1<<15,zp1_size=(1<<13)+(1<<12);
 zlib_buf_size=1<<i;
 while(1)
 {p=MemPtrNew(zlib_buf_size);zp0=MemPtrNew(zp0_size);
  zp1=MemPtrNew(zp1_size);
  if(!(p&&zp0&&zp1))
  {zlib_buf_size>>=3;
   if(p)MemPtrFree(p);if(zp0)MemPtrFree(zp0);break;if(zp1)MemPtrFree(zp1);break;
  }
  if(i==15)
  {char*_=MemPtrNew(zlib_buf_size);
   if(_){zlib_buf_size>>=2;MemPtrFree(_);}
   else zlib_buf_size>>=3;MemPtrFree(p);MemPtrFree(zp0);MemPtrFree(zp1);break;
  }MemPtrFree(p);MemPtrFree(zp0);MemPtrFree(zp1);
  i++;zlib_buf_size<<=1;
 }if(zlib_irrobust)zlib_buf_size<<=1;p=MemPtrNew(zlib_buf_size);
 if(!p)FrmAlert(Memory_Short_Alert_id);
 return p;
}static void
close_database(void){free_indices();close_dict_header(&dh);}
static int
decompress_content(unsigned rec_num)
{int cache_idx;
 if(!(dh.features&compression_bit)){uncompressed=*indices;return 0;}
 cache_idx=find_cache_item(rec_num,!0);
 if(cache_idx<0)return!0;return 0;
}static void
to_lower(char*dest,const char*src){StrToLower(dest,src);}
static int
compare_item(const char*wanted,const char*given,int n)
{return StrNCompare(wanted,given,n);}
enum bisect_result
{less_than_minus,more_than_plus,bisect_returns_range,bisect_error,
 bisect_try_next=1<<(sizeof(unsigned)*bits_per_byte-1)
};
static enum bisect_result
bisect(const char*title,int title_len,
 unsigned*lower,unsigned*upper)
{unsigned plus=idx_items_num()-1,minus=0,item,maxime,mixime;
 const unsigned*items=idx_items_array();int cmpp,vex=!0,cmp;
 cmp=compare_item(title,indices[1]+items[minus*3+2],title_len);
 if(facunde)
 {char s[0x33];
  StrIToA(s,minus);StrCat(s,":");StrIToA(s+StrLen(s),cmp);
  StrCat(s,"   ");draw_chars(s,0,28);
  draw_chars(indices[1]+items[minus*3+2],0,40);
  draw_chars(indices[1]+items[(minus+1)*3+2],80,40);
 }
 if(cmp<0){*lower=*upper=minus;return less_than_minus;}
 cmp=compare_item(title,indices[1]+items[plus*3+2],title_len);
 draw_chars(indices[1]+items[plus*3+2],80,40);
 if(cmp>0){*lower=*upper=plus;return more_than_plus;}
 maxime=mixime=plus;cmpp=-1;
 do
 {item=(plus+(unsigned long)minus)>>1;
  cmp=compare_item(title,indices[1]+items[item*3+2],title_len);
  if(cmp>0)minus=item;else
  {if(vex&&!cmp){vex=0;mixime=item;}if(cmp<0)maxime=item;
   plus=item;cmpp=cmp;
  }
  if(facunde)
  {char s[0x55];StrCopy(s,indices[1]+items[item*3+2]);
   draw_chars("   ",0,52);
   StrCat(s,":");StrIToA(s+StrLen(s),item);
   StrCat(s,":");StrIToA(s+StrLen(s),idx_items_num());
   StrCat(s,":");StrIToA(s+StrLen(s),minus);
   StrCat(s,":");StrIToA(s+StrLen(s),plus);
   StrCat(s,"     ");draw_chars(s,0,52);
  }
 }while(plus>minus+1);
 item=minus;if(!cmpp)item|=bisect_try_next;*lower=item;
 /*minus=mixime;plus=maxime;
 item=(plus+(unsigned long)minus)>>1;
 while(plus-1>minus)
 {cmp=compare_item(title,indices[1]+items[item*3+2],title_len);
  if(cmp<=0)plus=item;else minus=item;
  item=(plus+(unsigned long)minus)>>1;
 }item=plus;*upper=item;*/return bisect_returns_range;
}
static void
to_lower_n(char*dest,char*mid,const char*src,int n)
{StrNCopy(mid,src,n);mid[n]=0;to_lower(dest,mid);}
static enum bisect_result
bisect_article(const char*title,int title_len,
 unsigned*lower,unsigned*upper)
{unsigned plus=articles_number()-1,minus=0,item,maxime,mixime;
 const unsigned*items=((const unsigned*)uncompressed)+1;
 int cmp,vex=!0,cmpp,cmpm;enum bisect_result r=bisect_error;
 MemPtr given=MemPtrNew(title_len+3),copy=MemPtrNew(title_len+3);
 if(!(given&&copy)){*upper=*lower=minus;goto exit;}
 to_lower_n(given,copy,uncompressed+items[minus],title_len);
 cmp=compare_item(title,given,title_len);
 if(cmp<0)
 {*lower=*upper=minus;r=less_than_minus;
  draw_chars("ltm",80,64);
  goto exit;
 }cmpm=cmp;
 to_lower_n(given,copy,uncompressed+items[plus],title_len);
 draw_chars(given,70,76);draw_chars("arti",140,76);
 cmp=compare_item(title,given,title_len);cmpp=cmp;
 if(cmp>0)
 {*lower=*upper=plus;r=more_than_plus;
  draw_chars("mtp",80,76);
  goto exit;
 }maxime=mixime=plus;
 do
 {item=(plus+(unsigned long)minus)>>1;
  to_lower_n(given,copy,uncompressed+items[item],title_len);
  cmp=compare_item(title,given,title_len);
  if(cmp>0){minus=item;cmpm=cmp;}else
  {if(cmp<0)maxime=item;cmpp=cmp;
   plus=item;if(vex&&!cmp){vex=0;mixime=item;}
  }
  if(facunde)
  {char s[0x55];StrCopy(s,given);
   StrCat(s,":");StrIToA(s+StrLen(s),item);
   StrCat(s,":");StrIToA(s+StrLen(s),articles_number());
   StrCat(s,":");StrIToA(s+StrLen(s),minus);
   StrCat(s,":");StrIToA(s+StrLen(s),plus);
   StrCat(s,"     ");draw_chars(s,0,52);
  }
 }while(plus>minus+1);if(!cmpp)item=plus;else item=minus;
 *lower=item;minus=mixime;plus=maxime;
 /*while(plus-1>minus)
 {to_lower_n(given,copy,uncompressed+items[item],title_len-1);
  cmp=compare_item(title,given,title_len);
  if(cmp<0)plus=item;else minus=item;
  item=(plus+(unsigned long)minus)>>1;
 }if(cmp>0)item=plus;if(cmp==0)item=minus;*upper=item;*/
 r=bisect_returns_range;
 exit:if(given)MemPtrFree(given);if(copy)MemPtrFree(copy);return r;
}static int
save_uncompressed(void)
{saved_uncompressed=uncompressed;uncompressed=0;return 0;}
static void
discard_saved_uncompressed(void){saved_uncompressed=0;}
static int
restore_uncompressed(void)
{uncompressed=saved_uncompressed;saved_uncompressed=0;return 0;}
static unsigned last_line_shown,first_line_shown;
static int
find_article(const char*title)
{unsigned arity,lower=0,upper,try_next,prev_lower=0;
 unsigned title_len;enum bisect_result r;char*t;int ret=0;
 unsigned long tic=TimGetTicks();if(!dh.db)return!0;last_line_shown=0;
 if(!title||!*title)
 {current_article=0;current_content_record=first_record(0);
  if(*idx_handles)MemHandleUnlock(*idx_handles);
  *indices=0;*idx_handles=DmQueryRecord(dh.db,current_content_record);
  if(!*idx_handles)return!0;*indices=MemHandleLock(*idx_handles);
  if(!*indices)return!0;return decompress_content(current_content_record);
 }
 title_len=StrLen(title);t=MemPtrNew(title_len+1);
 if(!t)return-1;to_lower(t,title);
 unlock_handle(idx_handles+1);unlock_handle(idx_handles);
 *idx_handles=DmQueryRecord(dh.db,first_record(dh.ary));
 *indices=MemHandleLock(*idx_handles);
 for(arity=dh.ary,try_next=0;arity>0;arity--)
 {prev_lower=lower;
  {char s[0x33];StrCopy(s,"ary ");StrIToA(s+StrLen(s),arity);
   StrCat(s,"   ");draw_chars(s,0,0);
  }
  unlock_handle(idx_handles+1);
  idx_handles[1]=*idx_handles;indices[1]=*indices;
  r=bisect(t,title_len,&lower,&upper);
  switch(r)
  {case more_than_plus:
    if(try_next)
    {MemHandle h=idx_handles[1];MemPtr ptr=indices[1];unsigned low,up;
     idx_handles[1]=DmQueryRecord(dh.db,
      idx_items_array()[(prev_lower+1)*3+1]);
     indices[1]=MemHandleLock(idx_handles[1]);
     r=bisect(t,title_len,&low,&up);
     if(r==less_than_minus)
     {MemHandle h0=h;h=idx_handles[1];idx_handles[1]=h0;
      indices[1]=ptr;lower=low;
     }else{lower=low;upper=up;}
     MemHandleUnlock(h);
    }/*fall through*/
   case less_than_minus:/*??? rare case*/
   case bisect_returns_range:
    try_next=lower&bisect_try_next;lower&=~bisect_try_next;
    *idx_handles=DmQueryRecord(dh.db,idx_items_array()[lower*3+1]);
    *indices=MemHandleLock(*idx_handles);
    break;
   case bisect_error:/*this never occurs*/ret=-2;goto exit;
   default:
  }
 }if(decompress_content(idx_items_array()[lower*3+1]))goto exit;
 current_content_record=idx_items_array()[lower*3+1];
 prev_lower=lower;r=bisect_article(t,title_len,&lower,&upper);
 current_article=lower&~bisect_try_next;
 if(more_than_plus==r&&try_next
  &&current_content_record+1<first_record(1))
 {MemHandle h=*idx_handles;MemPtr ptr=*indices;int cmp;const unsigned*items;
  save_uncompressed();
  *idx_handles=DmQueryRecord(dh.db,current_content_record+1);
  *indices=MemHandleLock(*idx_handles);
  /*TODO test if there are 0 articles in the record*/
  if(decompress_content(current_content_record+1))
  {restore_uncompressed();MemHandleUnlock(*idx_handles);
   *indices=ptr;*idx_handles=h;goto exit;
  }
  items=((const unsigned*)uncompressed)+1;
  {MemPtr given=MemPtrNew(title_len+3),copy=MemPtrNew(title_len+3);
   if(!(given&&copy))
   {FrmAlert(Memory_Short_Alert_id);goto free_copies;}
   to_lower_n(given,copy,uncompressed+*items,title_len);
   cmp=compare_item(title,given,title_len);
   if(cmp)
   {MemHandle h0=*idx_handles;*idx_handles=h;h=h0;
    if(compression_bit&dh.features)restore_uncompressed();
    *indices=ptr;
   }else{current_content_record++;current_article=0;}
   discard_saved_uncompressed();MemHandleUnlock(h);draw_chars(given,0,91);
   free_copies:
   if(copy)MemPtrFree(copy);if(given)MemPtrFree(given);
  }
 }
exit:MemPtrFree(t);
 {char s[17];int sl;StrIToA(s,TimGetTicks()-tic);sl=StrLen(s);
  WinDrawChars(s,sl,screen_width-sl*5-2,status_line_y);
 }return ret;
}static int databases_num,db_idx;
struct database_handle**database_handles;
int
get_current_db_idx(void){return db_idx;}
struct database_handle**
get_database_list(int*items_number)
{if(items_number)*items_number=databases_num;return database_handles;}
static void
free_database_handles(void)
{if(databases_num)
 {int i;for(i=0;i<databases_num;i++)MemPtrFree(database_handles[i]);
  MemPtrFree(database_handles);database_handles=0;databases_num=0;
 }
}static int
list_databases(void)
{int n=0;DmSearchStateType ss;UInt16 card;LocalID id;
 struct db_name_chain{struct database_handle*h;struct db_name_chain*next;};
 struct db_name_chain*head=0;free_database_handles();
 if(DmGetNextDatabaseByTypeCreator(1,&ss,DB_TYPE,CREATOR,0,&card,&id))
  return n;
 do
 {struct db_name_chain*new_head;
  if(load_dict_header(&dh,card,id))continue;
  n++;new_head=MemPtrNew(sizeof*new_head);
  if(new_head)
  {struct database_handle*h=MemPtrNew(sizeof*h);
   if(h)
   {new_head->h=h;new_head->next=head;head=new_head;
    databases_num++;h->card=card;h->id=id;
    DmDatabaseInfo(card,id,h->name,0,0, 0,0,0, 0,0,0, 0,0);
    h->name[sizeof(h->name)-1]=0;
   }else MemPtrFree(new_head);
  }close_dict_header(&dh);
 }while(!DmGetNextDatabaseByTypeCreator(0,&ss,
          DB_TYPE,CREATOR,0,&card,&id));
 if(databases_num)
 {int i;database_handles=MemPtrNew(databases_num*sizeof*database_handles);
  for(i=0;head;i++)
  {struct db_name_chain*next=head->next;
   if(database_handles)database_handles[i]=head->h;
   else MemPtrFree(head->h);MemPtrFree(head);head=next;
  }if(!database_handles)databases_num=0;
 }return n;
}int
load_database(int index)
{if(index+1>databases_num||index<0)
 {char s[17],t[17];StrIToA(s,index);StrIToA(t,databases_num);
  FrmCustomAlert(Db_Index_Out_of_Range_Alert_id,s,t," ");
  return!0;
 }if(dh.db)close_database();
 if(load_dict_header(&dh,
     database_handles[index]->card,database_handles[index]->id))return!0;
 db_idx=index;current_content_record=first_record(0);return 0;
}
static void
clr_articles(void)
{static struct RectangleType r=
 {{x0,y0},{screen_width-x0,articles_height}};
 WinEraseRectangle(&r,0);
}static void
separate_articles(int y)
{WinDrawLine(5,y,screen_width/6,y);
 WinDrawLine(2*screen_width/6,y,3*screen_width/6,y);
 WinDrawLine(4*screen_width/6,y,5*screen_width/6,y);
 WinDrawLine(screen_width-5,y,screen_width,y);
}static void
mark_article_body(int x,int y,int dy)
{WinDrawLine(x,y,x,y+dy/3);WinDrawLine(x,y+dy*2/3,x,y+dy-1);
 WinDrawLine(x+2,y,x+2,y+dy/3);WinDrawLine(x+2,y+dy*2/3,x+2,y+dy-1);
 WinDrawLine(x,y,x+2,y);WinDrawLine(x,y+dy-1,x+2,y+dy-1);
}static int list_mode;
void
set_list_mode(int x){list_mode=x;}
int
get_list_mode(void){return list_mode;}
static int
delimiter(int c)
{if(c>='a'&&c<='z')return 0;if(c>='A'&&c<='Z')return 0;
 if(c>='0'&&c<='9')return 0;if(c<'0')return!0;if(c>'9'&&c<'A')return!0;
 if(c>'Z'&&c<'a')return!0;if(c>'z'&&c<0x80)return!0;return 0;
}static int
chars_in_width(const char*s,int width,int*wd,int*f)
{Int16 w=width,l=StrLen(s);Boolean fit;
 FntCharsInWidth(s,&w,&l,&fit);if(wd)*wd=w;if(f)*f=fit;return l;
}static int
reverse_font_word_wrap(const char*s0,int limit,int*wd,int m)
{char s[0x55];int f,n;const char*p;
 if(m>sizeof s-1)m=sizeof s-1;
 s[m]=0;for(m--;m>=0;m--)s[m]=s0[-1-m];
 n=chars_in_width(s,limit,wd,&f);if(f||!n)return n;
 for(p=s+n-1;!delimiter(*p)&&p!=s;p--);
 if(p==s)return n;n=p-s;
 if(wd)*wd=FntCharsWidth(s,n);return n;
}static int
font_word_wrap(const char*s,int limit,int*wd)
{int f,n=chars_in_width(s,limit,wd,&f);const char*p;
 if(f||!n)return n;
 for(p=s+n-1;!delimiter(*p)&&p!=s;p--);
 if(p==s)return n;n=p-s+1;
 if(wd)*wd=FntCharsWidth(s,s[n-1]?n:n-1);return n;
}static int
check_record_limit(unsigned*cur_rec,unsigned*art_num,int*saved)
{if(*art_num>=articles_number())
 {if(*cur_rec+1>=first_record(1))return!0;
  *art_num=0;(*cur_rec)++;
  if(!*saved){*saved=!0;save_uncompressed();}
  else MemHandleUnlock(*idx_handles);
  *idx_handles=DmQueryRecord(dh.db,*cur_rec);
  *indices=MemHandleLock(*idx_handles);
  if(decompress_content(*cur_rec))return!0;
 }return 0;
}static void
draw_ellipses(int y)
{int x=screen_width-1,dy=11;
 WinDrawPixel(x,y+2);WinDrawPixel(x,y+dy/2);WinDrawPixel(x,y+dy-3);
}static void
list_articles(unsigned*cur_rec,unsigned*art_num,int*saved)
{int x,y=y0,dy=11,y_max=y0+articles_height-dy,n,n0=0,width;
 while(y<=y_max)
 {const char*art;if(check_record_limit(cur_rec,art_num,saved))break;
  width=screen_width-x0;x=x0;
  art=article_ptr((*art_num)++);WinDrawChars(art,StrLen(art),x,y);
  x+=FntCharsWidth(art,StrLen(art));mark_article_body(x,y,dy);
  art+=StrLen(art)+2;art+=StrLen(art)+1;x+=4;width-=4;
  if(x>=screen_width){y+=dy;continue;}width-=x;
  {int wd;n=chars_in_width(art,width,&wd,0);
   WinDrawChars(art,art[n-1]?n:n-1,x,y);
   n0+=n;art+=n;if(*art)draw_ellipses(y);y+=dy;
  }
 }last_line_shown=0;
}static void
show_article(void)
{unsigned art_num=current_article,cur_rec=current_content_record,
  article_marked=!0;
 int x,y=y0,dy=11,y_max=y0+articles_height-dy,n,n0=0,
  width0=screen_width-x0-margin,width=width0,xb,vex=0;
 MemHandle rec=*idx_handles;MemPtr ptr=*indices;
 int saved=0;unsigned long tic=TimGetTicks();
 if(!dh.db)return;first_line_shown=last_line_shown;
 if(facunde)
 {char s[0x33];StrIToA(s,current_content_record);StrCat(s,":");
  StrIToA(s+StrLen(s),current_article);StrCat(s,":");
  StrIToA(s+StrLen(s),articles_number());StrCat(s,":");
  draw_chars(s,0,0);
 }clr_articles();x=x0;
 if(list_mode)list_articles(&cur_rec,&art_num,&saved);
 else while(y<=y_max)
 {const char*art,*art0;if(check_record_limit(&cur_rec,&art_num,&saved))break;
  art0=article_ptr(art_num++);art=art0+last_line_shown;
  if(!last_line_shown)
  {width-=2;WinDrawChars(art,StrLen(art),x,y);
   x+=FntCharsWidth(art,StrLen(art));art+=StrLen(art)+1;
   WinDrawChars(art,1,x,y);x+=FntCharsWidth(art++,1);
   if(!article_marked){separate_articles(y);article_marked=!0;}
   if(x+x0<width)width-=x;else
   {y+=dy;if(y>y_max)break;x=0;}
   do
   {n=font_word_wrap(art,width,0);
    WinDrawChars(art,art[n-1]?n:n-1,x,y);
    width=width0-2;n0+=n;y+=dy;last_line_shown=art-art0;
    xb=x+FntCharsWidth(art,art[n-1]?n:n-1);x=x0;art+=n;
   }while(*art&&y<=y_max);
   if(!*art)mark_article_body(xb,y-dy,dy);
   if(vex||!*art)last_line_shown=0;
   if(y>y_max){draw_ellipses(y-dy);break;}art++;
  }
  do
  {n=font_word_wrap(art,width,0);
   WinDrawChars(art,art[n-1]?n:n-1,x,y);
   y+=dy;n0+=n;x=x0;last_line_shown=art-art0;art+=n;
  }while(*art&&y<=y_max);article_marked=0;
  if(vex||!*art)last_line_shown=0;vex=!0;
  if(y>y_max&&*art)draw_ellipses(y-dy);
 }
 if(first_line_shown&&!list_mode)draw_ellipses(y0);
 if(saved)
 {unlock_handle(idx_handles);*idx_handles=rec;*indices=ptr;
  restore_uncompressed();
 }
 {char s[17];int sl;StrIToA(s,TimGetTicks()-tic);sl=StrLen(s);
  WinDrawChars(s,sl,screen_width-sl*5-30,status_line_y);
 }
 if(art_num+list_mode_inc_value>articles_number()
   &&(dh.features&compression_bit))
 {unsigned rn=cur_rec+1;MemHandle h=*idx_handles;MemPtr p=*indices;
  save_uncompressed();
  if(rn>=first_record(1))rn=first_record(0);
  *idx_handles=DmQueryRecord(dh.db,rn);
  if(*idx_handles)
  {*indices=MemHandleLock(*idx_handles);
   if(*indices)
   {find_cache_item(rn,0);
    MemHandleUnlock(*idx_handles);
   }
  }*idx_handles=h;*indices=p;restore_uncompressed();
 }
 if(art_num<list_mode_inc_value&&(dh.features&compression_bit))
 {unsigned rn=cur_rec;MemHandle h=*idx_handles;MemPtr p=*indices;
  save_uncompressed();
  if(rn<1)rn=first_record(1)-1;else rn--;
  *idx_handles=DmQueryRecord(dh.db,rn);
  if(*idx_handles)
  {*indices=MemHandleLock(*idx_handles);
   if(*indices)
   {find_cache_item(rn,0);
    MemHandleUnlock(*idx_handles);
   }
  }*idx_handles=h;*indices=p;restore_uncompressed();
 }
}static int
setup_zlib(void)
{if(!SysLibFind("Zlib",&ZLibRef))return 0;
 if(SysLibLoad('libr', 'ZLib',&ZLibRef))return!0;
 if(SysLibOpen(ZLibRef))return!0;return 0;
}static int
init(void)
{if(setup_zlib())ZLibRef=0;
 if(!list_databases())goto close_zlib;
 if(ZLibRef)
 {if(init_memory())goto close_zlib;
  if(init_cache())goto close_mem;
  zlib_buf=alloc_zlib_buf();if(!zlib_buf)goto close_cache;
 }
 init_show_battery();if(!load_preferences())return 0;
 close_cache:if(ZLibRef)close_cache();
 close_mem:if(ZLibRef)close_memory();
 close_zlib:if(ZLibRef){ZLTeardown;}return!0;
}static int current_form;FormType*form;
FormType*
get_current_form(void){return form;}
void
goto_form(int id)
{if(id<=0||id==current_form)return;
 current_form=id;FrmGotoForm(id);
}static Boolean
void_handler(EventType*e){return 0;}
static int
show_next_article(int increment)
{if(!dh.db)return!0;
 if(!list_mode&&!last_line_shown&&first_line_shown&&increment<0)
  first_line_shown=increment=0;
 if(!increment)
 {last_line_shown=first_line_shown;show_article();return 0;}
 if(last_line_shown&&!list_mode&&!(increment<0&&!first_line_shown))
 {if(increment<0)
  {const char*art0=article_ptr(current_article),*art=art0+first_line_shown,*b;
   int line,n,dy=11,first_line;
   b=art0+StrLen(art0)+1;b+=StrLen(b)+1;first_line=b-art0;
   for(line=0;line<articles_height/dy-1;line++)
   {n=reverse_font_word_wrap(art,screen_width-x0,0,first_line_shown-first_line);
    if(first_line_shown>n+first_line){first_line_shown-=n;art-=n;}
    else{first_line_shown=0;break;}
   }last_line_shown=first_line_shown;
  }
 }else if(increment<0)
 {increment=-increment;last_line_shown=0;
  if(current_article<increment)
  {MemHandleUnlock(*idx_handles);
   if(current_content_record==first_record(0))
    current_content_record=first_record(1)-1;
   else--current_content_record;
   *idx_handles=DmQueryRecord(dh.db,current_content_record);
   *indices=MemHandleLock(*idx_handles);
   if(decompress_content(current_content_record))return!0;
   if(articles_number()<=increment)current_article=0;
   else current_article=articles_number()-increment;
  }else current_article-=increment;
 }else
 {if(current_article+increment>=articles_number())
  {unsigned n=articles_number();
   MemHandleUnlock(*idx_handles);
   if(current_content_record+1>=first_record(1))
   {current_content_record=first_record(0);
   }else current_content_record++;
   *idx_handles=DmQueryRecord(dh.db,current_content_record);
   *indices=MemHandleLock(*idx_handles);
   if(decompress_content(current_content_record))return!0;
   current_article=current_article+increment-n;
   if(current_article>=articles_number())current_article=articles_number()-1;
  }else current_article+=increment;
 }show_article();return 0;
}static void
clr_status_line(void)
{static struct RectangleType r={{25,0},{160-25,9}};
 WinEraseRectangle(&r,0);
}static int skip_main_form_redraw;
void
skip_next_redraw(void){skip_main_form_redraw=!0;}
static void
on_enter_main_form(void)
{const char*s0=StrChr(dh.comment,'\n')+1;static int vex;
 int field_idx,show_art=skip_main_form_redraw;
 FrmDrawForm(form);show_battery(!0);
 clr_status_line();
 if(!vex)
 {char s[17];int sl;StrIToA(s,TimGetTicks()-global_ticks);sl=StrLen(s);
  WinDrawChars(s,sl,screen_width-sl*5-60,status_line_y);vex=!0;
 }
 if(s0)
 {const char*s1=StrChr(s0,'\n');
  if(s1)WinDrawChars(s0,s1-s0,25,status_line_y);
 }WinDrawLine(23,10,screen_width,10);
 FrmSetFocus(form,field_idx=FrmGetObjectIndex(form,LookupField_id));
 if(!skip_main_form_redraw)
 {FieldType*fl=FrmGetObjectPtr(form,field_idx);const char*s=get_lookup();
  FldDelete(fl,0,FldGetTextLength(fl));
  if(s&&StrLen(s))
  {FldInsert(fl,s,StrLen(s));
   FldSetSelection(fl,0,FldGetTextLength(fl));
  }show_art=!find_article(s);
 }skip_main_form_redraw=0;
 if(show_art)show_article();
}static int crosshair_x,crosshair_y,ch_shown;
static void
draw_crosshair(int x,int y)
{WinInvertLine(x-7,y-7,x+7,y+7);WinInvertLine(x+7,y-7,x-7,y+7);
 crosshair_x=x;crosshair_y=y;ch_shown=!ch_shown;
}static int
increment_value(void){return list_mode?list_mode_inc_value:1;}
static unsigned evt_loop_delay=289;
static Boolean
main_form_handler(EventType*e)
{static int saved_x,saved_y,saved;
 static unsigned long t0;unsigned long t=TimGetTicks();
 if(saved&&t-t0>17)
 {saved=0;
  if(list_mode)
  {if(ch_shown)draw_crosshair(crosshair_x,crosshair_y);
   if(saved_y>y0&&saved_y<y0+articles_height)draw_crosshair(saved_x,saved_y);
  }t0=t;evt_loop_delay=289;
 }
 switch(e->eType)
 {case frmOpenEvent:on_enter_main_form();break;
  case penMoveEvent:if(!list_mode)return 0;
   saved_x=e->screenX,saved_y=e->screenY;saved=!0;evt_loop_delay=10;break;
  case penUpEvent:
   if(e->screenY<y0)
   {if(!list_mode)list_mode=1;else list_mode--;
    if(ch_shown)draw_crosshair(crosshair_x,crosshair_y);
    show_article();break;
   }if(!list_mode)return 0;
   if(e->screenY>=y0+articles_height||e->screenY<=y0)return 0;
   if(ch_shown)draw_crosshair(crosshair_x,crosshair_y);
   {int dy=11,y=e->screenY-y0,inc;
    inc=y/dy;list_mode--;show_next_article(inc);
   }break;
  case penDownEvent:
   if(e->screenY<y0+articles_height&&(e->screenY>y0))
   {if(list_mode)draw_crosshair(e->screenX,e->screenX);
    else show_next_article(e->screenY<(screen_height-y0)/2?-1:1);
   }else return 0;break;
  case keyDownEvent:
   switch(((struct _KeyDownEventType*)(&(e->data)))->chr)
   {case vchrPageDown:show_next_article(increment_value());break;
    case vchrPageUp:show_next_article(-increment_value());break;
    case vchrFind:case '\n':
    {FieldType*f=(FieldType*)
      FrmGetObjectPtr(form,FrmGetObjectIndex(form,LookupField_id));
     if(!find_article(FldGetTextPtr(f)))show_article();
    }break;
    default:return 0;
   }break;
  case ctlSelectEvent:
   switch(e->data.ctlSelect.controlID){default:return 0;}break;
  case frmCloseEvent:
   {FieldType*fl=FrmGetObjectPtr(form,FrmGetObjectIndex(form,LookupField_id));
    const char*s=FldGetTextPtr(fl);set_lookup(s);
   }break;
  default:return 0;
 }return!0;
}static FormEventHandlerType*
handler(int form_id)
{switch(form_id)
 {case MainForm_id:return main_form_handler;
  case SettingsForm_id:return settings_form_handler;
 }return void_handler;
}static int
app_handle_event(EventType*e)
{switch(e->eType)
 {case keyDownEvent:
   if(((struct _KeyDownEventType*)(&(e->data)))->chr==vchrMenu)
   {if(current_form==MainForm_id)goto_form(SettingsForm_id);break;}
   return 0;
  case frmLoadEvent:form=FrmGetFormPtr(e->data.frmLoad.formID);
   if(!form)
   {form=FrmInitForm(e->data.frmLoad.formID);
    FrmSetEventHandler(form,handler(current_form));
   }
   FrmSetActiveForm(form);break;
  case appStopEvent:break;default:return 0;
 }return!0;
}static void
event_loop(void)
{while(1)
 {EventType e;EvtGetEvent(&e,evt_loop_delay);
  if(e.eType!=keyDownEvent||
   vchrFind!=((struct _KeyDownEventType*)(&(e.data)))->chr)
  if(SysHandleEvent(&e))continue;if(e.eType==appStopEvent)break;
  if(app_handle_event(&e))continue;FrmDispatchEvent(&e);show_battery(0);
 }
}static void(*on_close_current_form)(void);
void
at_close_app(void(*func)(void)){on_close_current_form=func;}
static void
close_all(void)
{if(current_form!=MainForm_id)
 {FrmEraseForm(form);FrmDeleteForm(form);
  if(on_close_current_form)on_close_current_form();
  form=FrmGetFormPtr(MainForm_id);
 }
 if(form)
 {FieldType*fl=FrmGetObjectPtr(form,FrmGetObjectIndex(form,LookupField_id));
  const char*s=FldGetTextPtr(fl);
  set_lookup(s);FrmEraseForm(form);FrmDeleteForm(form);
 }save_preferences();free_database_handles();close_database();
 if(ZLibRef)
 {close_cache();close_memory();if(zlib_buf)MemPtrFree(zlib_buf);
  ZLTeardown;
 }
}UInt32
PilotMain(UInt16 cmd,void*params,UInt16 flags)
{if(cmd!=sysAppLaunchCmdNormalLaunch)return 0;
 global_ticks=TimGetTicks();
 if(init()){SysTaskDelay(50);return 0;}
 goto_form(MainForm_id);event_loop();
 close_all();return 0;
}/*Copyright (C) 2008 Ineiev<ineiev@users.sourceforge.net>, super V 93

yepos is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.*/
