#include<vcl.h>
#pragma hdrstop

#include <libpokr/libpokrexp.h>
#include "gps.h"
#include <md5/md5.h>
#include <pokrpak/pack.h>

extern AnsiString informator_name;

cfg_t params;
std::string program_directory;

void* parse_section(char* ini_name,char* sections, char* section)
{
	void* var=0l;
	void* child=0l;
	char* cur=sections;
	char content[10000];

	while(*cur)
	{
		if(strncmp(section,cur,strlen(section))==0)
		if(cur[strlen(section)]=='\\'||section[0]==0)
		{
			char* next_name=0l;
			if(section[0]) next_name=cur+strlen(section)+1;
			else next_name=cur;

			if(*next_name)
			if(strchr(next_name,'\\')==0l)
			{
				child=parse_section(ini_name,sections,cur);
				if(child)
				{
					if(var==0l) var=pkr_create_struct();
					var=pkr_add_var(var,next_name,child);
					pkr_free_result(child);
				}
			}
		}

		cur+=strlen(cur)+1;
	}

	if(*section==0)
    return var;

	GetPrivateProfileSection(section,content,sizeof(content),ini_name);

	cur=content;
	while(*cur)
	{
		char* data=strchr(cur,'=');
		if(data)
		{
			*data=0;
			data++;
			if(*data)
			{
				if(*data=='"')
				{
					if(data[strlen(data)-1]=='"')
					{
						data++;
						data[strlen(data)-1]=0;
						if(var==0l) var=pkr_create_struct();
						var=pkr_add_strings(var,cur,data,1);
						cur=data+strlen(data)+2;
					}
					else cur=data+strlen(data)+1;
				}
				else if(strchr(cur,'.'))
				{
					if(var==0l) var=pkr_create_struct();
					var=pkr_add_double(var,cur,atof(data));
					cur=data+strlen(data)+1;
				}
				else
				{
					if(var==0l) var=pkr_create_struct();
					var=pkr_add_int(var,cur,atol(data));
					cur=data+strlen(data)+1;
				}
			}
			else cur+=2;
		}
		else cur+=strlen(cur)+1;
	}

	return var;
}

bool ParseIniFile()
{
	char sections[30000];

  AnsiString file=ExcludeTrailingPathDelimiter(ExtractFilePath(Application->ExeName))+"\\informator.ini";

	GetPrivateProfileSectionNames(sections,sizeof(sections),file.c_str());

	pkr_freezer pp(parse_section(file.c_str(),sections,""));
	if(pp.get()==0) return false;
  return params.unpack(pp.get());
}

VOID PrintError(HWND hParent,PCHAR szCaption,DWORD Error)
{
	LPVOID lpMsgBuf;
	FormatMessage( 
	    FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
	    NULL,
	    Error,
	    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
	    (LPTSTR) &lpMsgBuf,
	    0,
	    NULL 
	);
	MessageBox( hParent, (char*)lpMsgBuf, szCaption, MB_OK|MB_ICONERROR );
	LocalFree( lpMsgBuf );
}

void save_some(FILE* f,char* section,void* st)
{
	int i;
	int mi;
	void* var;
	char* name;

	mi=pkr_get_num_item(st);
	
	for(i=0;i<mi;i++)
	{
		char *var_name;

		var=pkr_get_item(st,i);
		var_name=pkr_get_item_name(st,i);

		switch(pkr_get_type(var))
		{
		case PKR_VAL_DOUBLE:
		case PKR_VAL_ONE_DOUBLE:
			fprintf(f,"%s=",var_name);
			if(pkr_get_num_item(var)>0) fprintf(f,"%e",pkr_get_double(var));
			fprintf(f,"\r\n");
			break;
		case PKR_VAL_INT:
		case PKR_VAL_ONE_INT:
			fprintf(f,"%s=",var_name);
			if(pkr_get_num_item(var)>0) fprintf(f,"%d",pkr_get_int(var));
			fprintf(f,"\r\n");
			break;
		case PKR_VAL_STR:
			fprintf(f,"%s=",var_name);
			if(pkr_get_num_item(var)>0) fprintf(f,"\"%s\"",pkr_get_data(var));
			fprintf(f,"\r\n");
			break;
		case PKR_VAL_STRUCT:
			name=(char*)malloc(strlen(section)+strlen(var_name)+2);
			strcpy(name,section);
			strcat(name,var_name);
			fprintf(f,"[%s]\r\n",name);
			strcat(name,"\\");
			save_some(f,name,var);
			free(name);
			break;
		}
	}
}

void save_ini()
{
  pkr_freezer fr(params.pack());
  AnsiString file=ExcludeTrailingPathDelimiter(ExtractFilePath(Application->ExeName))+"\\informator.ini";
	FILE* f=fopen(file.c_str(),"wb");
	if(f==0l) return;
	save_some(f,"",fr.get());
	fclose(f);
}


void add_old_param(void* &new_param,const void* old_param)
{
	int i;
	int mi=pkr_get_num_item(old_param);
	for(i=0;i<mi;i++)
	{
	  const char* name=pkr_get_item_name(old_param,i);
	  if(pkr_get_member(new_param,name)==0l)
	    new_param=pkr_add_var(new_param,name,pkr_get_item(old_param,i));
	}
}

void set_program_param(void* newpar)
{
  cfg_t cfg=params;
  cfg.unpack(newpar);

	if(pkr_get_member(newpar,"device"))
    infs.set_active(pkr_get_int(pkr_get_member(pkr_get_member(newpar,"device"),"dev_id")));

	set_server_param(cfg.server);
  open_server();

  if(pkr_get_member(newpar,"device"))dev_entry->device_params(pkr_get_member(newpar,"device"));
  if(pkr_get_member(newpar,"objects"))dev_entry->objects_params(pkr_get_member(newpar,"objects"));
  check_device_open();
  dev_entry->start();
  params=cfg;
  save_ini();
}

void set_auth_key()
{
	if(params.local.ask_login_password)
	{
		std::string login;
		std::string password;
		int i;

		if(!ask_login_password(login,password))
		{
			AddMessage("Работа с сервером: отсутствуют параметры аутентификации");
			return;
		}

    login+=password;

	  MD5_CTX md;
    MD5Init(&md);
    MD5Update(&md,(unsigned char*)login.c_str(),login.length());
    MD5Final(&md);

    key.clear();
		for(i=0;i<16;i++)
    {
      char buf[32];
      sprintf(buf,"%02x",md.digest[i]);
      key.insert(key.end(),reinterpret_cast<unsigned char*>(buf),reinterpret_cast<unsigned char*>(buf)+2);
    }
	}
	else
	{
		if(params.local.path_to_key.empty() )
		{
/*ORG			Application->MessageBox("Відсутні параметри аутентифікації.\r\n"
			"Вкажить їх за допомогою меню \"Параметри\\Аутентифікація\"\r\n",
	        "Аутентифікація",MB_OK|MB_ICONERROR);
*/
			Application->MessageBox(L"Відсутні параметри аутентифікації.\r\n"
		                       L"Вкажить їх за допомогою меню \"Параметри\\Аутентифікація\"\r\n",
                		       L"Аутентифікація",
		                       MB_OK|MB_ICONERROR);
			return;
		}

		FILE* f=fopen(params.local.path_to_key.c_str(),"rt");
		if(f==0l)
		{
/*ORG			Application->MessageBox("Неможливо відкрити файл з ключом.\r\n"
			"Вкажить його за допомогою меню \"Параметри\\Аутентифікація\"\r\n",
	        params.local.path_to_key.c_str(),MB_OK|MB_ICONERROR);
*/
			Application->MessageBox(L"Неможливо відкрити файл з ключом.\r\n"
	                      L"Вкажить його за допомогою меню \"Параметри\\Аутентифікація\"\r\n",
        	              UnicodeString(params.local.path_to_key.c_str()).w_str(),
	                      MB_OK|MB_ICONERROR);
                      return;
		}

		fseek(f,0,SEEK_END);
		int key_len=ftell(f);
		fseek(f,0,SEEK_SET);
		key.resize(key_len);
//ORG		fread(key.begin(),key_len,1,f);
		if (!key.empty())
		    fread(&key[0], key_len, 1, f);

		fclose(f);
	}
	
	return;
}

bool device_enabled=true;

void check_device_open()
{
  dev_entry->set_harware_enabled(device_enabled);
  if(!dev_entry->is_hardware_open()&&device_enabled)
    dev_entry->start();
}

//--cfg classes--------------------------------------------------------------

//--local_t------------------------------------------------------------------

local_t::local_t()
{
  ask_login_password=false;
  auto_update_cfg=true;

  log_program_message=true;
  log_device_exchange=true;
  write_device_exchange_time=true;
  log_sms=true;
  log_ip=true;
  log_synchro_packet=true;
  log_object=true;

  fix_gps_1024_weaks = true;
  fix_gps_tezey_device = false;
}

void* local_t::pack() const
{
  pkr_freezer d(pkr_create_struct());
  PKR_ADD_ITEM(path_to_key);
  PKR_ADD_ITEM(ask_login_password);
  PKR_ADD_ITEM(auto_update_cfg);

  PKR_ADD_ITEM(log_program_message);
  PKR_ADD_ITEM(log_device_exchange);
  PKR_ADD_ITEM(write_device_exchange_time);
  PKR_ADD_ITEM(log_sms);
  PKR_ADD_ITEM(log_ip);
  PKR_ADD_ITEM(log_synchro_packet);
  PKR_ADD_ITEM(log_object);

  PKR_ADD_ITEM(fix_gps_1024_weaks);
  PKR_ADD_ITEM(fix_gps_tezey_device);

  return d.replace(0);
}

bool local_t::unpack(void* d)
{
  return
    PKR_UNPACK(path_to_key)&&
    PKR_UNPACK(ask_login_password)&&
    PKR_UNPACK(auto_update_cfg)&&
    PKR_UNPACK(log_program_message)&&
    PKR_UNPACK(log_device_exchange)&&
    PKR_UNPACK(write_device_exchange_time)&&
    PKR_UNPACK(log_sms)&&
    PKR_UNPACK(log_ip)&&
    PKR_UNPACK(log_synchro_packet)&&
    PKR_UNPACK(log_object)&&
    PKR_UNPACK(fix_gps_1024_weaks)&&
	  PKR_UNPACK(fix_gps_tezey_device);
}

//--device_cfg_t-------------------------------------------------------------

device_cfg_t::device_cfg_t() : cdev_id(dev_id)
{
  dev_id=0;
  data=pkr_create_struct();
}

device_cfg_t::device_cfg_t(const device_cfg_t& rhs) : cdev_id(dev_id)
{
  dev_id=rhs.dev_id;
  data=rhs.data;
}

device_cfg_t& device_cfg_t::operator=(const device_cfg_t& rhs)
{
  dev_id=rhs.dev_id;
  data=rhs.data;
  return *this;
}

void* device_cfg_t::pack() const
{
  pkr_freezer ret;
  ret=data;

  return ret.replace(0);
}

bool device_cfg_t::unpack(void* d)
{
  if(!PKR_UNPACK(dev_id))return false;
  pkr_freezer cp(d);
  data=cp;
  cp.replace(0);
  return true;
}

//--objects_cfg_t------------------------------------------------------------
object_cfg_t::object_cfg_t()
{
  data=pkr_create_struct();
}

object_cfg_t::object_cfg_t(const object_cfg_t& rhs)
{
  data=rhs.data;
}

void* object_cfg_t::pack() const
{
  pkr_freezer ret;
  ret=data;
  return ret.replace(0);
}

bool object_cfg_t::unpack(void* d)
{
  pkr_freezer sp(d);
  data=sp;
  sp.replace(0);
  return true;
}

//--server_cfg_t-------------------------------------------------------------

server_cfg_t::server_cfg_t()
{
  inet_port=1503;
  is_tcp_cache=false;
  queue_len=50000000;
  informator_id=0;
}

void* server_cfg_t::pack() const
{
  pkr_freezer d(pkr_create_struct());
  PKR_ADD_ITEM(inet_host);
  PKR_ADD_ITEM(inet_port);
  PKR_ADD_ITEM(is_tcp_cache);
  PKR_ADD_ITEM(queue_len);
  PKR_ADD_ITEM(informator_id);
  PKR_ADD_ITEM(informator_name);
  return d.replace(0);
}

bool server_cfg_t::unpack(void* d)
{
  PKR_UNPACK(informator_name);
  return
    PKR_UNPACK(inet_host)&&
    PKR_UNPACK(inet_port)&&
    PKR_UNPACK(is_tcp_cache)&&
    PKR_UNPACK(queue_len)&&
    PKR_UNPACK(informator_id);
}

bool server_cfg_t::operator==(const server_cfg_t& rhs) const
{
  return
    inet_host==rhs.inet_host&&
    inet_port==rhs.inet_port&&
    is_tcp_cache==rhs.is_tcp_cache&&
    queue_len==rhs.queue_len&&
    informator_id==rhs.informator_id&&
    informator_name==rhs.informator_name;
}


//--cfg_t--------------------------------------------------------------------

void* cfg_t::pack() const
{
  pkr_freezer d(pkr_create_struct());
  PKR_ADD_ITEM(local);
  PKR_ADD_ITEM(device);
  PKR_ADD_ITEM(server);
  PKR_ADD_ITEM(objects);
  return d.replace(0);
}

bool cfg_t::unpack(void* d)
{
  return
    PKR_UNPACK(local)&&
    PKR_UNPACK(device)&&
    PKR_UNPACK(server)&&
    PKR_UNPACK(objects);
}


