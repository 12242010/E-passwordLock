#include "reg52.H"
#include "stdio.h"
#include "intrins.h"

unsigned char key_value;

#define DB P0//液晶并行数据接口
sbit  RS = P2^6;//液晶指令数据信号
sbit  RW = P2^5;//液晶读写信号
sbit  E  = P2^7;//液晶使能信号

sbit I2C_SCL = P2^3;//i2c时钟线定义
sbit I2C_SDA = P2^4;//i2c数据线定义

unsigned char now_window;//当前显示窗口
#define  chosemod  10//输入密码状态
#define  inputpassword  20//输入密码状态
#define  changepassword  30//输入密码状态
#define  putagain  31//再次输入密码状态
#define  keynoser  40//输入密码无效
#define  changeoldpassword  50//输入密码状态


unsigned char dis_dat[7]={0};
unsigned char password_now[7]={2,2,3,4,5,6};//输入当前密码保存数组
unsigned char password[7]={1,2,3,1,2,3};//密码保存
unsigned char chang_pass[7];//密码修改存储
signed 	 char curr_bit;//输入当前位
unsigned char curr_bit_tmp;//输入当前位备份
unsigned char i;
unsigned char cnt_100ms;
unsigned char time_100ms_flag;
unsigned char open_del;//开锁成功延时计数
unsigned char open_flag;//锁状态标志
unsigned char lock_flag;//锁状态标志

unsigned char password_err_cnt;//密码错误次数
unsigned int  err_tim_cnt;//密码错误次数
unsigned char cha_err_cnt;//修改密码错误延时计数
unsigned char cha_err_flag;//修改密码错误标志

unsigned char alm_tim_cnt;//报警时间计数
unsigned char alm_flag;//报警标志

sbit relay = P3^2;
sbit beep = P2^0;

/*******************************************************************************
* 函 数 名         : delay_ms
* 函数功能		   : 延时函数，延时1ms
* 输    入         : cnt
* 输    出         : 无
* 说    名         : 该函数是在12MHZ晶振下，12分频单片机的延时。
*******************************************************************************/
void delay_ms(unsigned int cnt)   //
{
	unsigned int x;
	for( ; cnt>0; cnt--)
	{
		for(x=110; x>0; x--);//软件延时为1MS
	}
}

/*******************************************************************************
* 函 数 名         : delay_us
* 函数功能		   : 延时函数，延时1us
* 输    入         : cnt
* 输    出         : 无
* 说    名         : 该函数是在12MHZ晶振下，12分频单片机的延时。
*******************************************************************************/
void delay_us(unsigned int cnt)   //
{
	while(cnt--);
}

/******************************************************
** 函数名：time_init
** 描述  ：定时器初始化
** 输入  ：无
** 输出  ：无
******************************************************/
void time_init(void)
{
	  TMOD |= 0x01;//time0 工作方式为1
	  TH0 = 0xf8;//装载初值
	  TL0 = 0x2f;//装载初值，为2ms(65535-63535)
      TR0 = 1;//开启定时器
	  ET0 = 1;//打开中断
	  EA=1;
}
/******************************************************
** 函数名：key_scan
** 描述  ：按键扫描
** 输入  ：无
** 输出  ：无
******************************************************/
void key_scan(void)
{
	static unsigned char key_in_flag = 0;//按键按下标志
	unsigned char key_l,key_h;//存储扫描到行列值。
	key_value = 20;//按键值清除
	P1 = 0xf0;
	if((P1 & 0xf0) != 0xf0)//按键按下
	{
		delay_ms(5);//按键消抖动
		if(((P1 & 0xf0) != 0xf0) && (key_in_flag == 1))
		{
			key_in_flag = 0;//松手检测防止一直触发
			key_l = P1;//扫描得到行值
			P1 = 0x0f;
			key_h= P1;////扫描得到列值
			switch(key_l|key_h)
			{
				//获取按键值
				case 0xee:  key_value = 1;break;
				case 0xed:  key_value = 2;break;
				case 0xeb:  key_value = 3;break;
				case 0xe7:  key_value = 10;break;

				case 0xde:  key_value = 4;break;
				case 0xdd:  key_value = 5;break;
				case 0xdb:  key_value = 6;break;
				case 0xd7:  key_value = 11;break;

				case 0xbe:  key_value = 7;break;
				case 0xbd:  key_value = 8;break;
				case 0xbb:  key_value = 9;break;
				case 0xb7:  key_value = 12;break;

				case 0x7e:  key_value = 13;break;
				case 0x7d:  key_value = 0;break;
				case 0x7b:  key_value = 14;break;
				case 0x77:  key_value = 15;break;
			}
		}
	}
	else
	{
		key_in_flag = 1;//(按键松开标志)
	}
}
/*******************************************************************************
* 函 数 名         : LcdWriteCom
* 函数功能		   : 向LCD写入一个字节的命令
* 输    入         : u8com
* 输    出         : 无
*******************************************************************************/
void lcd_wri_com(unsigned char com)	  //写入命令
{
	E = 0;	 //使能清零
	RS = 0;	 //选择写入命令
	RW = 0;	 //选择写入

	DB = com;
	delay_ms(1);//延时

	E = 1;	 //写入时序
	delay_ms(5);//延时
	E = 0;
}

/*******************************************************************************
* 函 数 名         : LcdWriteData
* 函数功能		   : 向LCD写入一个字节的数据
* 输    入         : u8dat
* 输    出         : 无
*******************************************************************************/

void lcd_wri_data(unsigned char dat)//写入数据
{
	E = 0;	  //使能清零
	RS = 1;	  //选择写入数据
	RW = 0;	  //选择写入
	DB = dat;
	delay_ms(1);//延时
	E = 1;	  //写入时序
	delay_ms(5);//延时
	E = 0;
}
/*******************************************************************************
* 函 数 名         : WriString
* 函数功能		   : 刷新屏幕显示
* 输    入         : hang，add，*p
* 输    出         : 无
*******************************************************************************/
void wri_string(unsigned char y,unsigned char x,unsigned char *p)
{
	if(y==1)//如果选择第一行
		lcd_wri_com(0x80+x);//选中地址
	else
		lcd_wri_com(0xc0+x);//选中地址
		while(*p)
		{
			lcd_wri_data(*p);//写入数据
			p++;
		}
}
/*******************************************************************************
* 函 数 名         : lcd_write_char
* 函数功能		   :
* 输    入         :
* 输    出         : 无
*******************************************************************************/
void lcd_write_char(unsigned char y, unsigned char x, unsigned char dat) //列x=0~15,行y=0,1
{
	unsigned char temp_l, temp_h;
	if(y==1)//如果选择第一行
		lcd_wri_com(0x80+x);//选中地址
	else
		lcd_wri_com(0xc0+x);//选中地址
	temp_l = dat % 10;
    temp_h = dat / 10;
    lcd_wri_data(temp_h + 0x30);//写入数据          //convert to ascii
    lcd_wri_data(temp_l + 0x30);//写入数据
}
/*********************光标控制***********************/
void lcd1602_cursor(unsigned char on_off,unsigned char add)
{
	if(on_off == 1)   //开光标
	{
		lcd_wri_com(0xc0+add);              //将光标移动到秒个位
		lcd_wri_com(0x0f);                //显示光标并且闪烁
	}
	else
	{
		lcd_wri_com(0x0c);   //关光标
	}
}

/*******************************************************************************
* 函 数 名       : LcdInit()
* 函数功能		 : 初始化LCD屏
* 输    入       : 无
* 输    出       : 无
*******************************************************************************/
void lcd_init(void)						  //LCD初始化子程序
{
	lcd_wri_com(0x38);//置功能8位双行
	lcd_wri_com(0x0c);//显示开关光标
	lcd_wri_com(0x06);//字符进入模式屏幕不动字符后移
	delay_ms(5);//延时5ms
	lcd_wri_com(0x01);  //清屏
	wri_string(1,0,"  welcome user  ");//初始化显示
	wri_string(2,0,"A:open  B:modify");//初始化显示
}
/*******************************************************************************
* 函 数 名         : I2C_Start()
* 函数功能		   : 起始信号：在I2C_SCL时钟信号在高电平期间I2C_SDA信号产生一个下降沿
* 输    入         : 无
* 输    出         : 无
* 备    注         : 起始之后I2C_SDA和I2C_SCL都为0
*******************************************************************************/
void I2C_Start()
{
	I2C_SCL = 1;//时钟线拉高
	delay_us(6);//延时
	I2C_SDA = 1;//数据线拉高
	delay_us(6);//延时
	I2C_SDA = 0;//数据线拉低
	delay_us(6);//延时
	I2C_SCL = 0;//时钟线拉低
	delay_us(6);//延时
}
/*******************************************************************************
* 函 数 名           : I2C_Stop()
* 函数功能	         : 终止信号：在I2C_SCL时钟信号高电平期间I2C_SDA信号产生一个上升沿
* 输    入           : 无
* 输    出         	 : 无
* 备    注           : 结束之后保持I2C_SDA和I2C_SCL都为1；表示总线空闲
*******************************************************************************/
void I2C_Stop()
{
	I2C_SCL = 0;//时钟线拉低
	delay_us(6);//延时
	I2C_SDA = 0;//数据线拉低
	delay_us(6);//延时
	I2C_SCL = 1;//时钟线拉高
	delay_us(6);//延时
	I2C_SDA = 1;//数据线拉高
	delay_us(6);//延时
}
void i2c_ACK(unsigned char ck)
{
    if (ck)
		I2C_SDA = 0;//数据线拉低
    else
		I2C_SDA = 1;//时钟线拉高
    delay_us(6);//延时
    I2C_SCL = 1;//时钟线拉高
    delay_us(6);//延时
    I2C_SCL = 0;//时钟线拉低
	delay_us(6);//延时
    I2C_SDA = 1;
    delay_us(6);//延时
}
unsigned char i2c_waitACK()
{
	I2C_SDA = 1;
	delay_us(6);//延时
	I2C_SCL = 1;//时钟线拉高
	delay_us(6);//延时
	if (I2C_SDA)
	{
		I2C_SCL = 0;//时钟线拉低
		I2C_Stop();
		return 1;
	}
	else
	{
		I2C_SCL = 0;//时钟线拉低
		return 0;
	}
}
void I2C_SendByte(unsigned char bt)
{
	unsigned char i;
    for(i=0; i<8; i++)
    {
        if (bt & 0x80)
			I2C_SDA = 1;
        else
			I2C_SDA = 0;//数据线拉低
        delay_us(6);//延时
        I2C_SCL = 1;//时钟线拉高
        bt <<= 1;
        delay_us(6);//延时
        I2C_SCL = 0;//时钟线拉低
    }
}
/*******************************************************************************
* 函 数 名           : I2cReadByte()
* 函数功能	    	 : 使用I2c读取一个字节
* 输    入           : 无
* 输    出         	 : dat
* 备    注           : 接收完一个字节I2C_SCL=0
*******************************************************************************/

unsigned char I2C_ReadByte()
{
		unsigned char dee, i;

		for (i=0; i<8; i++)
		{
			I2C_SCL = 1;//时钟线拉高
			delay_us(6);
			dee <<= 1;
			if (I2C_SDA)
				dee = dee | 0x01;
			I2C_SCL = 0;//时钟线拉低
			delay_us(6);
		}
		return dee;
}
/*******************************************************************************
* 函 数 名         : void At24c02Write(unsigned char u8addr,unsigned char u8dat)
* 函数功能		   : 往24c02的一个地址写入一个数据
* 输    入         : 无
* 输    出         : 无
*******************************************************************************/
unsigned char At24c02Write(unsigned char u8addr,unsigned char u8dat)
{
	I2C_Start();//起始信号
	I2C_SendByte(0xa0);//发送写器件地址
	if (i2c_waitACK())
		return 1;
	I2C_SendByte(u8addr);//发送要写入内存地址
	if (i2c_waitACK())
		return 1;
	I2C_SendByte(u8dat);//发送数据
	if (i2c_waitACK())
		return 1;
	I2C_Stop();//停止信号
	delay_ms(3);//延时
	return 0;
}
/*******************************************************************************
* 函 数 名         : unsigned char At24c02Read(unsigned char addr)
* 函数功能		   : 读取24c02的一个地址的一个数据
* 输    入         : 无
* 输    出         : 无
*******************************************************************************/

unsigned char At24c02Read(unsigned char u8addr,unsigned char *dat)
{

	I2C_Start();
	I2C_SendByte(0xa0); //发送写器件地址
	if (i2c_waitACK())
		return 1;
	I2C_SendByte(u8addr); //发送要读取的地址
	if (i2c_waitACK())
		return 1;
	I2C_Start();
	I2C_SendByte(0xa1); //发送读器件地址
	if (i2c_waitACK())
		return 1;
	*dat=I2C_ReadByte(); //读取数据
	i2c_ACK(0);
	I2C_Stop();
	return 0;
}
unsigned char At24c02WriteBuf(unsigned char *buf, unsigned char addr, unsigned char len)
{
	while (len--)
	{
		if (At24c02Write(addr++, *buf++))//连续写数据
			return 1;
	}
	return 0;
}
unsigned char At24c02ReadBuf(unsigned char *buf, unsigned char addr, unsigned char len)
{
	while (len--)
	{
		if (At24c02Read(addr++,buf++))//连续读取数据
		return 1;
	}
	return 0;
}
void At24c02init(void)//密码初始化
{
	unsigned char value;
	At24c02ReadBuf(&value,0x0a,1);
	if (value!=0xca)//第一次使用地址不是bb
	{
		value = 0xcc;
		At24c02WriteBuf(&value,0x0a,1);
		At24c02WriteBuf(password,0,6);
	}
	At24c02ReadBuf(password,0,6);
}
/******************************************************
 ** 函数名：time_service
 ** 描述  ：定时处理函数
 ** 输入  : 无
 ** 输出  ：无
 ** 调用  ：中断调用
 ******************************************************/
void time_service(void)
{
		if(time_100ms_flag)
		{
			time_100ms_flag = 0;
			if (alm_tim_cnt)
			alm_tim_cnt--;
			if (relay==0)//延时自动关锁
			{
				if(++open_del >= 10 * 5)	//10秒
				{
					open_del = 0;
				    relay = 1;   //关闭密码锁
				    now_window = chosemod;
					wri_string(1,0,"  welcome user  ");//初始化显示
					wri_string(2,0,"A:open  B:modify");//初始化显示
				}
			}
			if (open_flag)
			{
				if(alm_flag)
				{
					 if (alm_tim_cnt==0)
					 {
						 alm_flag = 0;
                	 	 alm_tim_cnt = 2;
						 beep = 1;
					 }
				}
				else
				{
					 if (alm_tim_cnt==0)
					 {
						 alm_flag = 1;
                	 	 alm_tim_cnt = 2;
						 beep = 0;
					 }
				}
				if(++open_del >= 10 * 2)
				{
					open_del = 0;
					open_flag = 0;
					alm_flag = 0;
				    alm_tim_cnt = 0;
				    beep = 1;
					if (password_err_cnt>=3)
					{
                        lock_flag = 1;
                        password_err_cnt = 0;
						return;
					}
					else
					{
						now_window = inputpassword;
						wri_string(1,0,"Input password  ");//
	                    wri_string(2,0,"                ");//
	                    lcd1602_cursor(1,0);
					}
				}
			}
			if (lock_flag)//错误三次
			{
					if(alm_flag)
					{
						 if (alm_tim_cnt==0)
						 {
							 alm_flag = 0;
	                	 	 alm_tim_cnt = 5;
							 beep = 1;
						 }
					}
					else
					{
						 if (alm_tim_cnt==0)
						 {
							 alm_flag = 1;
	                	 	 alm_tim_cnt = 5;
							 beep = 0;
						 }
					}
					wri_string(1,0,"  Machine lock  ");//初始化显示
					wri_string(2,0,"   one minute   ");//初始化显示
					lcd1602_cursor(0,0);
					now_window = keynoser;//按键不响应
					if(++err_tim_cnt>= 10 * 30)
					{

						now_window = chosemod;
                        err_tim_cnt = 0;
                        lock_flag = 0;
                        beep = 1;
						wri_string(1,0,"  welcome user  ");//初始化显示
						wri_string(2,0,"A:open  B:modify");//初始化显示
					}

			}
			if(cha_err_flag)//输入有误
			{
				if(alm_flag)
				{
					 if (alm_tim_cnt==0)
					 {
						 alm_flag = 0;
                	 	 alm_tim_cnt = 5;
						 beep = 1;
					 }
				}
				else
				{
					 if (alm_tim_cnt==0)
					 {
						 alm_flag = 1;
                	 	 alm_tim_cnt = 5;
						 beep = 0;
					 }
				}
				if (++cha_err_cnt>10*2)
				{
					cha_err_flag = 0;
					cha_err_cnt = 0;
					now_window = chosemod;
					wri_string(1,0,"  welcome user  ");//初始化显示
					wri_string(2,0,"A:open  B:modify");//初始化显示
				}
			}
		}
}
/******************************************************
 ** 函数名：key_service
 ** 描述  ：按键服务函数
 ** 输入  ：无
 ** 输出  ：无
 ** 调用  ：主程序
******************************************************/
void key_service(void)
{
        switch (now_window)
        {

              case chosemod:
              {
                     switch (key_value)
                     {
                            case 10://输入密码
                            {
                                wri_string(1,0,"Input password  ");//
                                wri_string(2,0,"                ");//
                                lcd1602_cursor(1,0);
								now_window = inputpassword;
                            }
                            break;
                            case 11://设置密码
                            {
                                wri_string(1,0,"Put Old Password ");//
	                            wri_string(2,0,"                 ");//
	                            lcd1602_cursor(1,0);
								now_window = changeoldpassword;
                            }
                            break;
                            case 12://设置密码
                            {
                                    password[0] = 1;
                                    password[1] = 1;
                                    password[2] = 1;
                                    password[3] = 1;
                                    password[4] = 1;
                                    password[5] = 1;
                                    At24c02WriteBuf(password,0,6);
									delay_ms(300);
									beep = 0;
									delay_ms(2000);
									beep = 1;
                            }
                            break;
                     }
              }
              break;
              case inputpassword://输入密码
              {
                        switch (key_value)
                        {
                        			case 1: case 2:case 3:
                        			case 4: case 5:case 6:
                        			case 7: case 8:case 9:case 0:
                        			{
                        				password_now[curr_bit] = key_value;
                        				if (++curr_bit>6)//输入6位密码
                        				{
                        					curr_bit = 6;
                        				}
                        				curr_bit_tmp = curr_bit;
                        				for(i=0;i<curr_bit_tmp;i++)//显示*号
                        				dis_dat[i] = '*';
										wri_string(2,0,dis_dat);
										lcd1602_cursor(1, curr_bit);
                        			}
        			                break;
        			                case 14://按下#删除密码
                        			{
                        				password_now[curr_bit] = 20;
                        				if (--curr_bit<0)
                        					curr_bit = 0;
										dis_dat[curr_bit] = ' ';//显示为空
										wri_string(2,0,dis_dat);
										lcd1602_cursor(1, curr_bit);
                        			}
        			                break;
                			        case 15://按下确认比对密码
                        			{
                        				if(
                        					password_now[0] == password[0]&&
                        				    password_now[1] == password[1]&&
                        					password_now[2] == password[2]&&
                        					password_now[3] == password[3]&&
                        					password_now[4] == password[4]&&
                        					password_now[5] == password[5]
                        				  )
                        				{

                        					relay = 0;//打开继电器
                        					wri_string(1,0,"    success    ");
											wri_string(2,0,"     open      ");
											lcd1602_cursor(0, curr_bit);
											now_window = keynoser;//按键不响应
                        				}
                        				else
                        				{
											  password_err_cnt++;
                        					  open_flag = 1;//错误标志
                        					  wri_string(1,0,"password err   ");
											  wri_string(2,0,"  err cnt:     ");
											  lcd_write_char(2,10,password_err_cnt);
											  lcd1602_cursor(0, curr_bit);
											  now_window = keynoser;//按键不响应
											  alm_flag = 1;
											  alm_tim_cnt = 4;
											  beep = 0;
                        				}
                        				for(i = 0;i < 6;i++)
                        				{
											  dis_dat[i] = 0;
	                        				  password_now[i] = 0;
	                        				  curr_bit = 0;
	                        				  curr_bit_tmp = 0;
                        				}
                        			}
                					break;
                		}
              }
              break;
              case changeoldpassword://修改密前输入旧密码
              {
                        switch (key_value)
                        {
                        			case 1: case 2:case 3:
                        			case 4: case 5:case 6:
                        			case 7: case 8:case 9:case 0:
                        			{
                        				password_now[curr_bit] = key_value;
                        				if (++curr_bit>6)//输入6位密码
                        				{
                        					curr_bit = 6;
                        				}
                        				curr_bit_tmp = curr_bit;
                        				for(i=0;i<curr_bit_tmp;i++)//显示*号
                        				dis_dat[i] = '*';
										wri_string(2,0,dis_dat);
										lcd1602_cursor(1, curr_bit);
                        			}
        			                break;
        			                case 14://按下#删除密码
                        			{
                        				password_now[curr_bit] = 20;
                        				if (--curr_bit<0)
                        					curr_bit = 0;
										dis_dat[curr_bit] = ' ';//显示为空
										wri_string(2,0,dis_dat);
										lcd1602_cursor(1, curr_bit);
                        			}
        			                break;
                			        case 15://按下确认比对密码
                        			{
                        				if(
                        					password_now[0] == password[0]&&
                        				    password_now[1] == password[1]&&
                        					password_now[2] == password[2]&&
                        					password_now[3] == password[3]&&
                        					password_now[4] == password[4]&&
                        					password_now[5] == password[5]
                        				  )
                        				{
                        					wri_string(1,0,"    success    ");
                                            delay_ms(500);
                                            wri_string(1,0,"Set New Password ");//
	                                        wri_string(2,0,"                 ");//
	                                        lcd1602_cursor(1,0);
											now_window = changepassword;//按键不响应
                        				}
                        				else
                        				{
                                              lcd1602_cursor(0,0);
                        					  wri_string(1,0,"password err    ");
                                              wri_string(2,0,"                ");
											  delay_ms(500);
                                              delay_ms(500);
                                              delay_ms(500);
                                              delay_ms(500);
                                              wri_string(1,0,"  welcome user  ");//初始化显示
					                          wri_string(2,0,"A:open  B:modify");//初始化显示
											  now_window = chosemod;

                        				}
                        				for(i = 0;i < 6;i++)
                        				{
											  dis_dat[i] = 0;
	                        				  password_now[i] = 0;
	                        				  curr_bit = 0;
	                        				  curr_bit_tmp = 0;
                        				}
                        			}
                					break;
                		}
              }
              break;
              case changepassword://修改密码
              {
					 switch (key_value)
					 {
							case 1: case 2:case 3:
                			case 4: case 5:case 6:
                			case 7: case 8:case 9:case 0:
                			{
                				password_now[curr_bit] = key_value;
                				if (++curr_bit>6)//输入6位密码
                				{
                					curr_bit = 6;
                				}
                				curr_bit_tmp = curr_bit;
                				for(i=0;i<curr_bit_tmp;i++)//显示*号
                				dis_dat[i] = '*';
								wri_string(2,0,dis_dat);
								lcd1602_cursor(1, curr_bit);
                			}
			                break;
						    case 14:
                			{
                				password_now[curr_bit] = 20;
                				if (--curr_bit<0)
                					curr_bit = 0;
								dis_dat[curr_bit] = ' ';//显示为空
								wri_string(2,0,dis_dat);
								lcd1602_cursor(1, curr_bit);
                			}
			                break;
							case 15:
							{

								for(i = 0;i < 6;i++)
                				{
								  dis_dat[i] = 0;
                				  curr_bit = 0;
                				  curr_bit_tmp = 0;
                				}
								wri_string(1,0,"   put again    ");//再次输入
	                            wri_string(2,0,"                ");//
	                            lcd1602_cursor(1,0);
								now_window = putagain;
							}
							break;
					 }
              }
              break;
			  case putagain://修改密码后再次输入
			  {
			  	 switch (key_value)
			  	 {
				 			case 1: case 2:case 3:
                			case 4: case 5:case 6:
                			case 7: case 8:case 9:case 0:
                			{
                				chang_pass[curr_bit] = key_value;
                				if (++curr_bit>6)//输入6位密码
                				{
                					curr_bit = 6;
                				}
                				curr_bit_tmp = curr_bit;
                				for(i=0;i<curr_bit_tmp;i++)//显示*号
                				dis_dat[i] = '*';
								wri_string(2,0,dis_dat);
								lcd1602_cursor(1, curr_bit);
                			}
			                break;
							case 14:
                			{
                				chang_pass[curr_bit] = 20;
                				if (--curr_bit<0)
                					curr_bit = 0;
								dis_dat[curr_bit] = ' ';//显示为空
								wri_string(2,0,dis_dat);
								lcd1602_cursor(1, curr_bit);
                			}
			                break;
							case 15:
							{


								if(
                        					password_now[0] == chang_pass[0]&&
                        				    password_now[1] == chang_pass[1]&&
                        					password_now[2] == chang_pass[2]&&
                        					password_now[3] == chang_pass[3]&&
                        					password_now[4] == chang_pass[4]&&
                        					password_now[5] == chang_pass[5]
                        		  )
								{
									wri_string(1,0,"change Password ");
									wri_string(2,0,"    Succeed     ");
									for(i=0;i<6;i++)
									{
										password[i] = chang_pass[i];   //保存密码
									}
									At24c02WriteBuf(password,0,6);
									delay_ms(300);
									beep = 0;
									delay_ms(2000);
									beep = 1;
									now_window = chosemod;
									wri_string(1,0,"  welcome user  ");//初始化显示
									wri_string(2,0,"A:open  B:modify");//初始化显示
								}
								else
								{
									    cha_err_flag = 1;
										wri_string(1,0," Two different  ");
										wri_string(2,0,"     return     ");
										now_window = keynoser;//按键不响应
										alm_flag = 1;
										alm_tim_cnt = 5;
										beep = 0;
								}
								for(i = 0;i < 6;i++)
                				{
								  dis_dat[i] = 0;
                				  password_now[i] = 0;
                				  curr_bit = 0;
                				  curr_bit_tmp = 0;
                				}

							}
							break;

				 }
			  }
			  break;
        }


}
/******************************************************
 ** 函数名：init_all_hardware
 ** 描述  ：初始化所有硬件，及其变量参数。
 ** 输入  ：无
 ** 输出  ：无
 ** 调用  ：主程序
 ******************************************************/
void init_all_hardware(void)
{
   		delay_ms(100);
		time_init();//定时器初始化
        lcd_init();//液晶初始化
		At24c02init();//存储初始化
		key_value = 20;//默认没按键按下
		now_window = chosemod;//选择模式
		cnt_100ms = 0;
		time_100ms_flag = 0;
		open_del = 0;
		relay = 1;//继电器关闭。
		curr_bit = 0;
		open_flag = 0;
		password_err_cnt =0;
		err_tim_cnt = 0;
		alm_tim_cnt = 0;
		alm_flag = 0;
		cha_err_flag = 0;
		cha_err_cnt = 0;
        beep = 1;
        lock_flag =0;
}
void main(void)
{
	 init_all_hardware();//初始化硬件，IO和定时器
	 while(1)
	 {
	 	 key_scan();//按键扫描
		 key_service();//按键服务处理函数
		 time_service();//时间处理函数
	 }
}
 /******************************************************
 ** 函数名：time0_interrupt
 ** 描述  ：按键扫描函数
 ** 输入  ：无
 ** 输出  ：无
 ******************************************************/
void time0_interrupt() interrupt 1
{
	   TF0 = 0;//清除标志
	   TR0 = 0;
	   if (++cnt_100ms>50)
	   {
			cnt_100ms = 0;
			time_100ms_flag = 1;
	   }
	   TR0 = 1;
	   TH0 = 0xf8;
	   TL0 = 0x2f;//装载初值2ms(65535-63535)
}
