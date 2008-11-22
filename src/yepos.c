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
enum local_constants
{bits_per_byte=8,screen_width=160,screen_height=160,
 x0=0,y0=11,status_line_y=-2,articles_height=screen_height-y0-12,
 log2_cache_length=4,cache_length=1<<log2_cache_length,
 cache_length_mask=cache_length-1
};
enum local_defines{facunde=0,zlib_irrobust=1};
static void
draw_chars(const char*s,int x,int y)
{if(facunde)WinDrawChars(s,StrLen(s),x,y);
 if(facunde>1)SysTaskDelay(50);
}
static unsigned*features,*record_size,*ary,*volumes,
 *vol,*ary_records,rec0_size,comment_size;
static char*db_comment;
static DmOpenRef current_db;static MemHandle record0;
static unsigned zlib_buf_size;static char*zlib_buf;
static int
parse_header(int verbous)
{char s[0x33];char*p;unsigned*up;int y=0,incy=11,i,j;
 record0=DmQueryRecord(current_db,0);
 if(!record0)return!0;p=MemHandleLock(record0);if(!p)return!0;
 rec0_size=MemPtrSize(p);
 up=(unsigned*)p;features=up;
 if(*features&~implemented_features_bits)
 {FrmAlert(Unimplemented_Alert_id);return!0;}
 if(*features&compression_bit&&!ZLibRef)
 {FrmAlert(No_Zlib_Alert_id);return!0;}
 record_size=up+1;ary=up+2;
 volumes=up+3;vol=up+4;ary_records=up+5;
 db_comment=(char*)(ary_records+*ary+1);
 comment_size=rec0_size-(db_comment-p);
 if(!verbous)return 0;
 StrCopy(s,"Database ");j=StrLen(s);
 for(i=0;i<comment_size;i++,j++)
 {s[j]=db_comment[i];
  if(s[j]=='\n')
  {s[j]=0;WinDrawChars(s,StrLen(s),0,y);y+=incy;j=-1;}
 }
 if(db_comment[i-1]!='\n')
 {s[j]=0;WinDrawChars(s,StrLen(s),0,y);y+=incy;}
 StrCopy(s,"features: ");StrIToH(s+StrLen(s),*features);
 StrCat(s,"; arity: ");StrIToA(s+StrLen(s),*ary);
 WinDrawChars(s,StrLen(s),0,y);y+=incy;
 StrCopy(s,"content record size: ");
 StrIToA(s+StrLen(s),*record_size);
 WinDrawChars(s,StrLen(s),0,y);y+=incy;
 StrCopy(s,"volumes: ");StrIToA(s+StrLen(s),*volumes);
 StrCat(s,"; current volume: ");StrIToA(s+StrLen(s),*vol);
 WinDrawChars(s,StrLen(s),0,y);y+=incy;
 for(i=0;i<=*ary;i++)
 {StrIToA(s,i);StrCat(s,"-ary records begin with ");
  StrIToA(s+StrLen(s),ary_records[i]);
  WinDrawChars(s,StrLen(s),0,y);y+=incy;
 }return 0;
}MemHandle*idx_handles;
struct mem_chunk uncompressed_chunk;
static const char*uncompressed;char**indices;
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
 {p->chunk.d=invalid_chunk_descriptor;p->content=0;p->db_idx=0;p->rec_num=0;}
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
  if(err){zlib_error_alert(err,"inflateInit");goto exit;}
  while(decompressed<orig_size)
  {unsigned n;str.next_out=zlib_buf;str.avail_out=zlib_buf_size;
   err=inflate(&str,0);
   if(err&&err!=Z_STREAM_END){zlib_error_alert(err,"inflate");goto exit;}
   n=zlib_buf_size-str.avail_out;
   write_chunk(m,decompressed,zlib_buf,n);decompressed+=n;
  }while(err!=Z_STREAM_END);
  err=inflateEnd(&str);
  if(err)zlib_error_alert(err,"inflateEnd");
 exit:
  uncompressed=lock_chunk(m);return err;
}static int
find_cache_item(unsigned rec_num)
{int i=cache_head,d=get_current_db_idx(),idle_item=(cache_head+1)&cache_length_mask;
 if(facunde)
 {char s[0x33];StrCopy(s,"cache find ");StrCat(s,"(");
   StrIToA(s+StrLen(s),d);StrCat(s,":");
   StrIToA(s+StrLen(s),rec_num);StrCat(s,")");
   draw_chars(s,0,0);
 }
 do
 {
  if(facunde)
  {char s[0x33];StrCopy(s,"cache ");StrIToA(s+StrLen(s),i);StrCat(s,":(");
   StrIToA(s+StrLen(s),cache[i].db_idx);StrCat(s,":");
   StrIToA(s+StrLen(s),cache[i].rec_num);StrCat(s,");");
   StrIToA(s+StrLen(s),cache[i].chunk.d);StrCat(s,"  ");
   draw_chars(s,0,0);
  }
  if(cache[i].chunk.d<0){idle_item=i;goto next_i;}
  if(d==cache[i].db_idx&&rec_num==cache[i].rec_num)
  {uncompressed=cache[i].content;return i;}
  next_i:i=(i-1)&cache_length_mask;
 }while(i!=cache_head);
 if(facunde)
 {char s[0x33];StrCopy(s,"cache loop done ");StrIToA(s+StrLen(s),idle_item);StrCat(s,":(");
   StrIToA(s+StrLen(s),cache[idle_item].db_idx);StrCat(s,":");
   StrIToA(s+StrLen(s),cache[idle_item].rec_num);StrCat(s,");");
   StrIToA(s+StrLen(s),cache[idle_item].chunk.d);StrCat(s,"  ");
   draw_chars(s,0,0);
 }
 if(cache[idle_item].chunk.d>=0)free_chunk(cache[idle_item].chunk);
 cache[idle_item].chunk.d=invalid_chunk_descriptor;
 i=(idle_item+1)&cache_length_mask;
 while(i!=idle_item)
 {cache[idle_item].chunk=alloc_chunk(*record_size);
  if(cache[idle_item].chunk.d>=0){cache_head=idle_item;break;}
  while(i!=idle_item)
  {int i_prev=i;i=(i+1)&cache_length_mask;
   if(cache[i_prev].chunk.d>=0)
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
  {cache[idle_item].rec_num=rec_num;cache[idle_item].db_idx=d;
   cache[idle_item].content=uncompressed;
   return idle_item;
  }
 }return-1;
}
static unsigned
idx_items_num(int arity){return*((unsigned*)(indices[arity])+1);}
static const unsigned*
idx_items_array(int arity){return((unsigned*)(indices[arity]))+2;}
static unsigned
first_record(int arity){return ary_records[arity];}
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
{if(indices)MemPtrFree(indices);indices=0;
 if(idx_handles)
 {int i;
  for(i=0;i<MemPtrSize(idx_handles)/sizeof(*idx_handles);i++)
   unlock_handle(idx_handles+i);
  MemPtrFree(idx_handles);idx_handles=0;
 }
 if(uncompressed_chunk.d>=0)
 {free_chunk(uncompressed_chunk);
  uncompressed_chunk.d=-1;uncompressed=0;
 }current_article=0;current_content_record=1;
 close_cache();
}static int
alloc_indices(void)
{int vex=0;uncompressed_chunk.d=-1;
 indices=MemPtrNew(sizeof(*indices)*(*ary+1));
 vex|=!indices;
 idx_handles=MemPtrNew(sizeof(*idx_handles)*(*ary+1));
 vex|=!idx_handles;
 if(!vex)current_content_record=first_record(0);
 if(vex)FrmAlert(Memory_Short_Alert_id);
 return vex;
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
close_database(void)
{free_indices();unlock_handle(&record0);
 if(current_db){DmCloseDatabase(current_db);current_db=0;}
}
static int
decompress_content(unsigned rec_num)
{int cache_idx;
 if(!(*features&compression_bit)){uncompressed=indices[0];return 0;}
 uncompressed=0;cache_idx=find_cache_item(rec_num);
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
bisect(const char*title,int title_len,int arity,
 unsigned*lower,unsigned*upper)
{unsigned plus=idx_items_num(arity)-1,minus=0,item,maxime,mixime;
 const unsigned*items=idx_items_array(arity);int cmpp,vex=!0,cmp;
 cmp=compare_item(title,indices[arity]+items[minus*3+2],title_len);
 if(facunde)
 {char s[0x33];StrIToA(s,arity);StrCat(s,":");
  StrIToA(s+StrLen(s),minus);StrCat(s,":");StrIToA(s+StrLen(s),cmp);
  StrCat(s,"   ");draw_chars(s,0,28);
  draw_chars(indices[arity]+items[minus*3+2],0,40);
  draw_chars(indices[arity]+items[(minus+1)*3+2],80,40);
 }
 if(cmp<0){*lower=*upper=minus;return less_than_minus;}
 cmp=compare_item(title,indices[arity]+items[plus*3+2],title_len);
 if(0)draw_chars(indices[arity]+items[plus*3+2],80,40);
 if(cmp>0){*lower=*upper=plus;return more_than_plus;}
 maxime=mixime=plus;cmpp=-1;
 do
 {item=(plus+(unsigned long)minus)>>1;
  cmp=compare_item(title,indices[arity]+items[item*3+2],title_len);
  if(cmp>0)minus=item;else
  {if(vex&&!cmp){vex=0;mixime=item;}if(cmp<0)maxime=item;
   plus=item;cmpp=cmp;
  }
  if(facunde)
  {char s[0x55];StrCopy(s,indices[arity]+items[item*3+2]);
   draw_chars("   ",0,52);
   StrCat(s,":");StrIToA(s+StrLen(s),arity);
   StrCat(s,":");StrIToA(s+StrLen(s),item);
   StrCat(s,":");StrIToA(s+StrLen(s),idx_items_num(arity));
   StrCat(s,":");StrIToA(s+StrLen(s),minus);
   StrCat(s,":");StrIToA(s+StrLen(s),plus);
   StrCat(s,"     ");draw_chars(s,0,52);
  }
 }while(plus>minus+1);
 item=minus;if(!cmpp)item|=bisect_try_next;*lower=item;
 /*minus=mixime;plus=maxime;
 item=(plus+(unsigned long)minus)>>1;
 while(plus-1>minus)
 {cmp=compare_item(title,indices[arity]+items[item*3+2],title_len);
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
   StrCat(s,"     ");WinDrawChars(s,StrLen(s),0,52);
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
}
static struct mem_chunk saved_uncomp_h;static const char*saved_uncompressed;
static int
save_uncompressed(void)
{if(compression_bit&*features)
 {saved_uncomp_h=uncompressed_chunk;uncompressed_chunk.d=-1;}
 saved_uncompressed=uncompressed;uncompressed=0;return 0;
}
static void
discard_saved_uncompressed(void)
{if(compression_bit&*features)
 {free_chunk(saved_uncomp_h);saved_uncomp_h.d=-1;}
 saved_uncompressed=0;
}static int
restore_uncompressed(void)
{if(compression_bit&*features)
 {free_chunk(uncompressed_chunk);
  uncompressed_chunk=saved_uncomp_h;saved_uncomp_h.d=-1;
 }uncompressed=saved_uncompressed;saved_uncompressed=0;return 0;
}static int
find_article(const char*title)
{unsigned arity,lower=0,upper,try_next,prev_lower=0;
 unsigned title_len;enum bisect_result r;char*t;int ret=0;
 unsigned long tic=TimGetTicks();if(!current_db)return!0;
 title_len=StrLen(title);t=MemPtrNew(title_len+1);
 if(!t)return-1;to_lower(t,title);
 for(arity=*ary,try_next=0;arity>0;arity--)
 {prev_lower=lower;
  r=bisect(t,title_len,arity,&lower,&upper);
  switch(r)
  {case more_than_plus:
    if(try_next)
    {MemHandle h=idx_handles[arity];MemPtr ptr=indices[arity];
     unsigned low,up;
     idx_handles[arity]=DmQueryRecord(current_db,
      idx_items_array(arity+1)[(prev_lower+1)*3+1]);
     indices[arity]=MemHandleLock(idx_handles[arity]);
     r=bisect(t,title_len,arity,&low,&up);
     if(r==less_than_minus)
     {MemHandle h0=h;h=idx_handles[arity];idx_handles[arity]=h0;
      indices[arity]=ptr;lower=low;
     }else{lower=low;upper=up;}
     MemHandleUnlock(h);
    }/*fall through*/
   case less_than_minus:/*???*/
   case bisect_returns_range:
    try_next=lower&bisect_try_next;lower&=~bisect_try_next;
    if(idx_handles[arity-1])MemHandleUnlock(idx_handles[arity-1]);
    idx_handles[arity-1]=DmQueryRecord(current_db,
     idx_items_array(arity)[lower*3+1]);
    indices[arity-1]=MemHandleLock(idx_handles[arity-1]);
    break;
   case bisect_error:/*this never occurs*/ret=-2;goto exit;
   default:
  }
 }if(decompress_content(idx_items_array(1)[lower*3+1]))goto exit;
 current_content_record=idx_items_array(1)[lower*3+1];
 prev_lower=lower;r=bisect_article(t,title_len,&lower,&upper);
 current_article=lower&~bisect_try_next;
 if(more_than_plus==r&&try_next
  &&current_content_record+1<first_record(1))
 {MemHandle h=*idx_handles;MemPtr ptr=*indices;
  int cmp;const unsigned*items;
  save_uncompressed();
  *idx_handles=DmQueryRecord(current_db,
   current_content_record+1);
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
    if(compression_bit&*features)restore_uncompressed();
    *indices=ptr;
   }else{current_content_record++;current_article=0;}
   if(*features&compression_bit)if(saved_uncomp_h.d>=0)
    discard_saved_uncompressed();
   MemHandleUnlock(h);
   draw_chars(given,0,91);
   free_copies:
   if(copy)MemPtrFree(copy);if(given)MemPtrFree(given);
  }
 }
exit:MemPtrFree(t);
 {char s[17];int sl;StrIToA(s,TimGetTicks()-tic);sl=StrLen(s);
  WinDrawChars(s,sl,screen_width-sl*5-2,status_line_y);
 }return ret;
}int databases_num,db_idx;struct database_handle**database_handles;
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
}struct db_name_chain{struct database_handle*h;struct db_name_chain*next;};
static void
clr_scrn(void)
{static struct RectangleType r={{0,0},{160,160}};
 WinEraseRectangle(&r,0);
}static int
list_databases(void)
{int n=0;DmSearchStateType ss;UInt16 card;LocalID id;
 struct db_name_chain*head=0;free_database_handles();
 if(DmGetNextDatabaseByTypeCreator(1,&ss,DB_TYPE,CREATOR,0,&card,&id))
  return n;
 do
 {struct db_name_chain*new_head;
  if(current_db)DmCloseDatabase(current_db);
  current_db=DmOpenDatabase(0,id,dmModeReadOnly);
  if(!current_db)break;
  clr_scrn();if(parse_header(!0))continue;
  n++;new_head=MemPtrNew(sizeof*new_head);
  if(new_head)
  {struct database_handle*h=MemPtrNew(sizeof*h);
   if(h)
   {new_head->h=h;new_head->next=head;head=new_head;
    databases_num++;h->card=card;h->id=id;
    DmDatabaseInfo(card,id,h->name,0,0, 0,0,0, 0,0,0, 0,0);
    h->name[sizeof(h->name)-1]=0;
   }else MemPtrFree(new_head);
  }
  MemHandleUnlock(record0);DmCloseDatabase(current_db);current_db=0;
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
{int r=!0,i;if(index+1>databases_num)return!0;
 if(current_db)close_database();
 current_db=DmOpenDatabase(database_handles[index]->card,
  database_handles[index]->id,dmModeReadOnly);
 if(!current_db)
 {char s[0x33];StrCopy(s,"fail opening ");
  WinDrawChars(s,StrLen(s),0,148);
  return!0;
 }
 if(parse_header(0)||alloc_indices()){close_database();return r;}
 for(i=0;i<=*ary;i++)
 {idx_handles[i]=DmQueryRecord(current_db,ary_records[i]);
  indices[i]=MemHandleLock(idx_handles[i]);
 }db_idx=index;return decompress_content(current_content_record=first_record(0));
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
font_word_wrap(const char*s,int limit,int*wd)
{int f,n=chars_in_width(s,limit,wd,&f);const char*p;
 if(f||!n)return n;
 for(p=s+n-1;!delimiter(*p)&&p!=s;p--);
 if(p==s)return n;n=p-s+1;
 if(wd)*wd=FntCharsWidth(s,s[n-1]?n:n-1);return n;
}
static void
show_article(void)
{unsigned art_num=current_article,cur_rec=current_content_record,
  article_marked=!0;
 int x,y=y0,dy=11,y_max=y0+articles_height-dy,n,n0=0,
  width=screen_width-x0,xb;
 MemHandle rec=*idx_handles;MemPtr ptr=*indices;
 int saved=0;unsigned long tic=TimGetTicks();
 if(!current_db)return;
 if(facunde)
 {char s[0x33];StrIToA(s,current_content_record);StrCat(s,":");
  StrIToA(s+StrLen(s),current_article);StrCat(s,":");
  StrIToA(s+StrLen(s),articles_number());StrCat(s,":");
  draw_chars(s,0,0);
 }clr_articles();x=x0;
 while(y<y_max)/*TODO show long articles appropriately*/
 {const char*art;
  if(art_num>=articles_number())
  {if(cur_rec+1>=first_record(1))break;
   art_num=0;cur_rec++;
   if(!saved){saved=!0;save_uncompressed();}
   else MemHandleUnlock(*idx_handles);
   *idx_handles=DmQueryRecord(current_db,cur_rec);
   *indices=MemHandleLock(*idx_handles);
   if(decompress_content(cur_rec))break;
  }
  art=article_ptr(art_num++);
  WinDrawChars(art,StrLen(art),x,y);
  x+=FntCharsWidth(art,StrLen(art));art+=StrLen(art)+1;
  if(!list_mode){WinDrawChars(art,1,x,y);x+=FntCharsWidth(art++,1);}
  else{art++;art+=StrLen(art)+1;}
  if(list_mode){mark_article_body(x,y,dy);x+=4;width-=4;}
  if(x<screen_width)width-=x;else
  {if(!(article_marked||list_mode)){separate_articles(y);article_marked=!0;}
   y+=dy;if(y>y_max)break;if(!list_mode)x=0;
  }
  do
  {int wd;
   n=list_mode?chars_in_width(art,width,&wd,0):font_word_wrap(art,width,&wd);
   WinDrawChars(art,art[n-1]?n:n-1,x,y);
   if(!article_marked){separate_articles(y);article_marked=!0;}
   width=screen_width;n0+=n;y+=dy;
   xb=x+FntCharsWidth(art,art[n-1]?n:n-1);x=x0;art+=n;
  }while(*art&&y<y_max&&!list_mode);
  if(y>y_max)break;if(list_mode)continue;
  mark_article_body(xb,y-dy,dy);art++;
  do
  {n=font_word_wrap(art,width,0);
   WinDrawChars(art,art[n-1]?n:n-1,x,y);
   y+=dy;n0+=n;x=x0;art+=n;
  }while(*art&&y<y_max);article_marked=0;
 }
 if(saved)
 {unlock_handle(idx_handles);*idx_handles=rec;*indices=ptr;
  restore_uncompressed();
 }
 {char s[17];int sl;StrIToA(s,TimGetTicks()-tic);sl=StrLen(s);
  WinDrawChars(s,sl,screen_width-sl*5-30,status_line_y);
 }
}/*static void
process_database(LocalID id)
{MemHandle content_rec;MemPtr content;
 current_db=DmOpenDatabase(0,id,dmModeReadOnly);
 if(parse_header(!0))goto bad_header;
 if(compression_bit&*features)
 {unsigned i,n=ary_records[1];char s[0x88],*st;
  unsigned long tic0=0,max_tics=0;int err=0;
  StrCopy(s,"Rec ");st=s+StrLen(s);
  zlib_buf=alloc_zlib_buf();
  if(zlib_buf)
  {for(i=ary_records[0];i<n;i++)
   {unsigned long tic1=TimGetTicks();unsigned cont_size,*orig_size;
    char*cont;uLongf dest_len;z_stream str;
    MemSet(&str,sizeof str,0);
    content_rec=DmQueryRecord(current_db,i);
    content=MemHandleLock(content_rec);
    cont=((char*)content)+2;orig_size=(unsigned*)content;
    dest_len=*orig_size;cont_size=MemPtrSize(content)-2;
    str.next_out=uncompressed;
    str.avail_out=*record_size;
    str.next_in=cont;
    str.avail_in=cont_size;
    err=inflateInit(&str);
    if(err)zlib_error_alert(err,"inflateInit");
    if(!err)
    {err=inflate(&str,Z_FINISH);
     if(err!=Z_STREAM_END)zlib_error_alert(err,"inflate");
     err=err!=Z_STREAM_END;
    }
    if(!err)
    {err=inflateEnd(&str);
     if(err)zlib_error_alert(err,"inflateEnd");
    }
    MemHandleUnlock(content_rec);
    tic1=TimGetTicks()-tic1;
    if(max_tics<tic1)max_tics=tic1;
    tic0+=tic1;
    StrIToA(st,i);StrCat(st,": dt ");StrIToA(st+StrLen(st),tic1);
    StrCat(st,"; ");StrIToA(st+StrLen(st),max_tics);
    StrCat(st,"; ");StrIToA(st+StrLen(st),tic0);StrCat(st,"; err ");
    StrIToA(st+StrLen(st),(int)err);StrCat(st,"          ");
    WinDrawChars(s,StrLen(s),0,130);
    StrIToA(st,cont_size);StrCat(st,":");StrIToA(st+StrLen(st),dest_len);
    StrCat(st,"   ");WinDrawChars(st,StrLen(st),0,142);
    if(err)break;
   }MemPtrFree(uncompressed);uncompressed=0;
  }
 }else
 {unsigned i,ptr_size,*orig_size,*articles,*tptr;char*cont;
  char s[0x33],*st;uLongf dest_len;
  content_rec=DmQueryRecord(current_db,ary_records[0]);
  content=MemHandleLock(content_rec);
  dest_len=*orig_size;ptr_size=MemPtrSize(content);
  cont=(char*)content;
  articles=(unsigned*)content;tptr=articles+1;
  StrCopy(s,"articles: ");StrIToA(s,articles[0]);
  StrCat(s,"   ");WinDrawChars(s,StrLen(s),0,110);
  StrCopy(s,"title 0: `");
  st=s+StrLen(s);
  for(i=0;i<5&&cont[tptr[0]+i];i++)*st++=cont[tptr[0]+i];
  *st=0;StrCat(s,"'");
  StrCat(st,"   ");WinDrawChars(s,StrLen(s),0,122);
  MemHandleUnlock(content_rec);
 }
bad_header:MemHandleUnlock(record0);DmCloseDatabase(current_db);current_db=0;
}*/
static int
setup_zlib(void)
{if(!SysLibFind("Zlib",&ZLibRef))return 0;
 if(SysLibLoad('libr', 'ZLib',&ZLibRef))return!0;
 if(SysLibOpen(ZLibRef))return!0;return 0;
}
static int
init_statum(void)
{load_preferences();if(current_db)return 0;
 if(!databases_num)
 {FrmAlert(No_Dictionary_Alert_id);return!0;}
 /*process_database(id);*/
 if(load_database(0))
 {WinDrawChars("can't load database",19,10,0);return!0;}
 return 0;
}static int
init(void)
{if(setup_zlib())ZLibRef=0;
 if(!list_databases())goto close_zlib;
 if(ZLibRef)
 {if(init_memory())goto close_zlib;
  if(init_cache())goto close_mem;
  zlib_buf=alloc_zlib_buf();if(!zlib_buf)goto close_cache;
 }
 init_show_battery();
 if(!init_statum())return 0;
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
{if(!current_db)return!0;if(!increment){show_article();return 0;}
 if(increment<0)
 {increment=-increment;
  if(current_article<increment)
  {MemHandleUnlock(idx_handles[0]);
   if(current_content_record==first_record(0))
    current_content_record=first_record(1)-1;
   else--current_content_record;
   *idx_handles=DmQueryRecord(current_db,current_content_record);
   *indices=MemHandleLock(*idx_handles);
   if(decompress_content(current_content_record))return!0;
   if(articles_number()<=increment)current_article=0;
   else current_article=articles_number()-increment;
  }else current_article-=increment;
 }else
 {if(current_article+increment>=articles_number())
  {unsigned n=articles_number();
   MemHandleUnlock(idx_handles[0]);
   if(current_content_record+1>=first_record(1))
   {current_content_record=first_record(0);
   }else current_content_record++;
   *idx_handles=DmQueryRecord(current_db,current_content_record);
   *indices=MemHandleLock(idx_handles[0]);
   if(decompress_content(current_content_record))return!0;
   current_article=current_article+increment-n;
   if(current_article>=articles_number())current_article=articles_number()-1;
  }else current_article+=increment;
 }show_article();return 0;
}static void
clr_status_line(void)
{static struct RectangleType r={{25,0},{160-25,9}};
 WinEraseRectangle(&r,0);
}static void
on_enter_main_form(void)
{const char*s0=StrChr(db_comment,'\n')+1;int field_idx;
 FrmDrawForm(form);show_battery(!0);clr_status_line();
 if(s0)
 {const char*s1=StrChr(s0,'\n');
  if(s1)WinDrawChars(s0,s1-s0,25,status_line_y);
 }WinDrawLine(23,10,screen_width,10);
 FrmSetFocus(form,field_idx=FrmGetObjectIndex(form,LookupField_id));
 {FieldType*fl=FrmGetObjectPtr(form,field_idx);const char*s=get_lookup();
  if(s&&StrLen(s))
  {FldDelete(fl,0,FldGetTextLength(fl));
   FldInsert(fl,s,StrLen(s));find_article(s);
   FldSetSelection(fl,0,FldGetTextLength(fl));
  }
 }show_article();
}static int crosshair_x,crosshair_y,ch_shown;
static void
draw_crosshair(int x,int y)
{WinInvertLine(x-7,y-7,x+7,y+7);WinInvertLine(x+7,y-7,x-7,y+7);
 crosshair_x=x;crosshair_y=y;ch_shown=!ch_shown;
}static int
increment_value(void){return list_mode?7:1;}
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
}static void
close_all(void)
{if(current_form!=MainForm_id)
 {FrmEraseForm(form);FrmDeleteForm(form);
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
