#include "reg52.H"
#include "stdio.h"
#include "intrins.h"

unsigned char key_value;

#define DB P0//Һ���������ݽӿ�
sbit  RS = P2^6;//Һ��ָ�������ź�
sbit  RW = P2^5;//Һ����д�ź�
sbit  E  = P2^7;//Һ��ʹ���ź�

sbit I2C_SCL = P2^3;//i2cʱ���߶���
sbit I2C_SDA = P2^4;//i2c�����߶���

unsigned char now_window;//��ǰ��ʾ����
#define  chosemod  10//��������״̬
#define  inputpassword  20//��������״̬
#define  changepassword  30//��������״̬
#define  putagain  31//�ٴ���������״̬
#define  keynoser  40//����������Ч
#define  changeoldpassword  50//��������״̬


unsigned char dis_dat[7]={0};
unsigned char password_now[7]={2,2,3,4,5,6};//���뵱ǰ���뱣������
unsigned char password[7]={1,2,3,1,2,3};//���뱣��
unsigned char chang_pass[7];//�����޸Ĵ洢
signed 	 char curr_bit;//���뵱ǰλ
unsigned char curr_bit_tmp;//���뵱ǰλ����
unsigned char i;
unsigned char cnt_100ms;
unsigned char time_100ms_flag;
unsigned char open_del;//�����ɹ���ʱ����
unsigned char open_flag;//��״̬��־
unsigned char lock_flag;//��״̬��־

unsigned char password_err_cnt;//����������
unsigned int  err_tim_cnt;//����������
unsigned char cha_err_cnt;//�޸����������ʱ����
unsigned char cha_err_flag;//�޸���������־

unsigned char alm_tim_cnt;//����ʱ�����
unsigned char alm_flag;//������־

sbit relay = P3^2;
sbit beep = P2^0;

/*******************************************************************************
* �� �� ��         : delay_ms
* ��������		   : ��ʱ��������ʱ1ms
* ��    ��         : cnt
* ��    ��         : ��
* ˵    ��         : �ú�������12MHZ�����£�12��Ƶ��Ƭ������ʱ��
*******************************************************************************/
void delay_ms(unsigned int cnt)   //
{
	unsigned int x;
	for( ; cnt>0; cnt--)
	{
		for(x=110; x>0; x--);//�����ʱΪ1MS
	}
}

/*******************************************************************************
* �� �� ��         : delay_us
* ��������		   : ��ʱ��������ʱ1us
* ��    ��         : cnt
* ��    ��         : ��
* ˵    ��         : �ú�������12MHZ�����£�12��Ƶ��Ƭ������ʱ��
*******************************************************************************/
void delay_us(unsigned int cnt)   //
{
	while(cnt--);
}

/******************************************************
** ��������time_init
** ����  ����ʱ����ʼ��
** ����  ����
** ���  ����
******************************************************/
void time_init(void)
{
	  TMOD |= 0x01;//time0 ������ʽΪ1
	  TH0 = 0xf8;//װ�س�ֵ
	  TL0 = 0x2f;//װ�س�ֵ��Ϊ2ms(65535-63535)
      TR0 = 1;//������ʱ��
	  ET0 = 1;//���ж�
	  EA=1;
}
/******************************************************
** ��������key_scan
** ����  ������ɨ��
** ����  ����
** ���  ����
******************************************************/
void key_scan(void)
{
	static unsigned char key_in_flag = 0;//�������±�־
	unsigned char key_l,key_h;//�洢ɨ�赽����ֵ��
	key_value = 20;//����ֵ���
	P1 = 0xf0;
	if((P1 & 0xf0) != 0xf0)//��������
	{
		delay_ms(5);//����������
		if(((P1 & 0xf0) != 0xf0) && (key_in_flag == 1))
		{
			key_in_flag = 0;//���ּ���ֹһֱ����
			key_l = P1;//ɨ��õ���ֵ
			P1 = 0x0f;
			key_h= P1;////ɨ��õ���ֵ
			switch(key_l|key_h)
			{
				//��ȡ����ֵ
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
		key_in_flag = 1;//(�����ɿ���־)
	}
}
/*******************************************************************************
* �� �� ��         : LcdWriteCom
* ��������		   : ��LCDд��һ���ֽڵ�����
* ��    ��         : u8com
* ��    ��         : ��
*******************************************************************************/
void lcd_wri_com(unsigned char com)	  //д������
{
	E = 0;	 //ʹ������
	RS = 0;	 //ѡ��д������
	RW = 0;	 //ѡ��д��

	DB = com;
	delay_ms(1);//��ʱ

	E = 1;	 //д��ʱ��
	delay_ms(5);//��ʱ
	E = 0;
}

/*******************************************************************************
* �� �� ��         : LcdWriteData
* ��������		   : ��LCDд��һ���ֽڵ�����
* ��    ��         : u8dat
* ��    ��         : ��
*******************************************************************************/

void lcd_wri_data(unsigned char dat)//д������
{
	E = 0;	  //ʹ������
	RS = 1;	  //ѡ��д������
	RW = 0;	  //ѡ��д��
	DB = dat;
	delay_ms(1);//��ʱ
	E = 1;	  //д��ʱ��
	delay_ms(5);//��ʱ
	E = 0;
}
/*******************************************************************************
* �� �� ��         : WriString
* ��������		   : ˢ����Ļ��ʾ
* ��    ��         : hang��add��*p
* ��    ��         : ��
*******************************************************************************/
void wri_string(unsigned char y,unsigned char x,unsigned char *p)
{
	if(y==1)//���ѡ���һ��
		lcd_wri_com(0x80+x);//ѡ�е�ַ
	else
		lcd_wri_com(0xc0+x);//ѡ�е�ַ
		while(*p)
		{
			lcd_wri_data(*p);//д������
			p++;
		}
}
/*******************************************************************************
* �� �� ��         : lcd_write_char
* ��������		   :
* ��    ��         :
* ��    ��         : ��
*******************************************************************************/
void lcd_write_char(unsigned char y, unsigned char x, unsigned char dat) //��x=0~15,��y=0,1
{
	unsigned char temp_l, temp_h;
	if(y==1)//���ѡ���һ��
		lcd_wri_com(0x80+x);//ѡ�е�ַ
	else
		lcd_wri_com(0xc0+x);//ѡ�е�ַ
	temp_l = dat % 10;
    temp_h = dat / 10;
    lcd_wri_data(temp_h + 0x30);//д������          //convert to ascii
    lcd_wri_data(temp_l + 0x30);//д������
}
/*********************������***********************/
void lcd1602_cursor(unsigned char on_off,unsigned char add)
{
	if(on_off == 1)   //�����
	{
		lcd_wri_com(0xc0+add);              //������ƶ������λ
		lcd_wri_com(0x0f);                //��ʾ��겢����˸
	}
	else
	{
		lcd_wri_com(0x0c);   //�ع��
	}
}

/*******************************************************************************
* �� �� ��       : LcdInit()
* ��������		 : ��ʼ��LCD��
* ��    ��       : ��
* ��    ��       : ��
*******************************************************************************/
void lcd_init(void)						  //LCD��ʼ���ӳ���
{
	lcd_wri_com(0x38);//�ù���8λ˫��
	lcd_wri_com(0x0c);//��ʾ���ع��
	lcd_wri_com(0x06);//�ַ�����ģʽ��Ļ�����ַ�����
	delay_ms(5);//��ʱ5ms
	lcd_wri_com(0x01);  //����
	wri_string(1,0,"  welcome user  ");//��ʼ����ʾ
	wri_string(2,0,"A:open  B:modify");//��ʼ����ʾ
}
/*******************************************************************************
* �� �� ��         : I2C_Start()
* ��������		   : ��ʼ�źţ���I2C_SCLʱ���ź��ڸߵ�ƽ�ڼ�I2C_SDA�źŲ���һ���½���
* ��    ��         : ��
* ��    ��         : ��
* ��    ע         : ��ʼ֮��I2C_SDA��I2C_SCL��Ϊ0
*******************************************************************************/
void I2C_Start()
{
	I2C_SCL = 1;//ʱ��������
	delay_us(6);//��ʱ
	I2C_SDA = 1;//����������
	delay_us(6);//��ʱ
	I2C_SDA = 0;//����������
	delay_us(6);//��ʱ
	I2C_SCL = 0;//ʱ��������
	delay_us(6);//��ʱ
}
/*******************************************************************************
* �� �� ��           : I2C_Stop()
* ��������	         : ��ֹ�źţ���I2C_SCLʱ���źŸߵ�ƽ�ڼ�I2C_SDA�źŲ���һ��������
* ��    ��           : ��
* ��    ��         	 : ��
* ��    ע           : ����֮�󱣳�I2C_SDA��I2C_SCL��Ϊ1����ʾ���߿���
*******************************************************************************/
void I2C_Stop()
{
	I2C_SCL = 0;//ʱ��������
	delay_us(6);//��ʱ
	I2C_SDA = 0;//����������
	delay_us(6);//��ʱ
	I2C_SCL = 1;//ʱ��������
	delay_us(6);//��ʱ
	I2C_SDA = 1;//����������
	delay_us(6);//��ʱ
}
void i2c_ACK(unsigned char ck)
{
    if (ck)
		I2C_SDA = 0;//����������
    else
		I2C_SDA = 1;//ʱ��������
    delay_us(6);//��ʱ
    I2C_SCL = 1;//ʱ��������
    delay_us(6);//��ʱ
    I2C_SCL = 0;//ʱ��������
	delay_us(6);//��ʱ
    I2C_SDA = 1;
    delay_us(6);//��ʱ
}
unsigned char i2c_waitACK()
{
	I2C_SDA = 1;
	delay_us(6);//��ʱ
	I2C_SCL = 1;//ʱ��������
	delay_us(6);//��ʱ
	if (I2C_SDA)
	{
		I2C_SCL = 0;//ʱ��������
		I2C_Stop();
		return 1;
	}
	else
	{
		I2C_SCL = 0;//ʱ��������
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
			I2C_SDA = 0;//����������
        delay_us(6);//��ʱ
        I2C_SCL = 1;//ʱ��������
        bt <<= 1;
        delay_us(6);//��ʱ
        I2C_SCL = 0;//ʱ��������
    }
}
/*******************************************************************************
* �� �� ��           : I2cReadByte()
* ��������	    	 : ʹ��I2c��ȡһ���ֽ�
* ��    ��           : ��
* ��    ��         	 : dat
* ��    ע           : ������һ���ֽ�I2C_SCL=0
*******************************************************************************/

unsigned char I2C_ReadByte()
{
		unsigned char dee, i;

		for (i=0; i<8; i++)
		{
			I2C_SCL = 1;//ʱ��������
			delay_us(6);
			dee <<= 1;
			if (I2C_SDA)
				dee = dee | 0x01;
			I2C_SCL = 0;//ʱ��������
			delay_us(6);
		}
		return dee;
}
/*******************************************************************************
* �� �� ��         : void At24c02Write(unsigned char u8addr,unsigned char u8dat)
* ��������		   : ��24c02��һ����ַд��һ������
* ��    ��         : ��
* ��    ��         : ��
*******************************************************************************/
unsigned char At24c02Write(unsigned char u8addr,unsigned char u8dat)
{
	I2C_Start();//��ʼ�ź�
	I2C_SendByte(0xa0);//����д������ַ
	if (i2c_waitACK())
		return 1;
	I2C_SendByte(u8addr);//����Ҫд���ڴ��ַ
	if (i2c_waitACK())
		return 1;
	I2C_SendByte(u8dat);//��������
	if (i2c_waitACK())
		return 1;
	I2C_Stop();//ֹͣ�ź�
	delay_ms(3);//��ʱ
	return 0;
}
/*******************************************************************************
* �� �� ��         : unsigned char At24c02Read(unsigned char addr)
* ��������		   : ��ȡ24c02��һ����ַ��һ������
* ��    ��         : ��
* ��    ��         : ��
*******************************************************************************/

unsigned char At24c02Read(unsigned char u8addr,unsigned char *dat)
{

	I2C_Start();
	I2C_SendByte(0xa0); //����д������ַ
	if (i2c_waitACK())
		return 1;
	I2C_SendByte(u8addr); //����Ҫ��ȡ�ĵ�ַ
	if (i2c_waitACK())
		return 1;
	I2C_Start();
	I2C_SendByte(0xa1); //���Ͷ�������ַ
	if (i2c_waitACK())
		return 1;
	*dat=I2C_ReadByte(); //��ȡ����
	i2c_ACK(0);
	I2C_Stop();
	return 0;
}
unsigned char At24c02WriteBuf(unsigned char *buf, unsigned char addr, unsigned char len)
{
	while (len--)
	{
		if (At24c02Write(addr++, *buf++))//����д����
			return 1;
	}
	return 0;
}
unsigned char At24c02ReadBuf(unsigned char *buf, unsigned char addr, unsigned char len)
{
	while (len--)
	{
		if (At24c02Read(addr++,buf++))//������ȡ����
		return 1;
	}
	return 0;
}
void At24c02init(void)//�����ʼ��
{
	unsigned char value;
	At24c02ReadBuf(&value,0x0a,1);
	if (value!=0xca)//��һ��ʹ�õ�ַ����bb
	{
		value = 0xcc;
		At24c02WriteBuf(&value,0x0a,1);
		At24c02WriteBuf(password,0,6);
	}
	At24c02ReadBuf(password,0,6);
}
/******************************************************
 ** ��������time_service
 ** ����  ����ʱ������
 ** ����  : ��
 ** ���  ����
 ** ����  ���жϵ���
 ******************************************************/
void time_service(void)
{
		if(time_100ms_flag)
		{
			time_100ms_flag = 0;
			if (alm_tim_cnt)
			alm_tim_cnt--;
			if (relay==0)//��ʱ�Զ�����
			{
				if(++open_del >= 10 * 5)	//10��
				{
					open_del = 0;
				    relay = 1;   //�ر�������
				    now_window = chosemod;
					wri_string(1,0,"  welcome user  ");//��ʼ����ʾ
					wri_string(2,0,"A:open  B:modify");//��ʼ����ʾ
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
			if (lock_flag)//��������
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
					wri_string(1,0,"  Machine lock  ");//��ʼ����ʾ
					wri_string(2,0,"   one minute   ");//��ʼ����ʾ
					lcd1602_cursor(0,0);
					now_window = keynoser;//��������Ӧ
					if(++err_tim_cnt>= 10 * 30)
					{

						now_window = chosemod;
                        err_tim_cnt = 0;
                        lock_flag = 0;
                        beep = 1;
						wri_string(1,0,"  welcome user  ");//��ʼ����ʾ
						wri_string(2,0,"A:open  B:modify");//��ʼ����ʾ
					}

			}
			if(cha_err_flag)//��������
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
					wri_string(1,0,"  welcome user  ");//��ʼ����ʾ
					wri_string(2,0,"A:open  B:modify");//��ʼ����ʾ
				}
			}
		}
}
/******************************************************
 ** ��������key_service
 ** ����  ������������
 ** ����  ����
 ** ���  ����
 ** ����  ��������
******************************************************/
void key_service(void)
{
        switch (now_window)
        {

              case chosemod:
              {
                     switch (key_value)
                     {
                            case 10://��������
                            {
                                wri_string(1,0,"Input password  ");//
                                wri_string(2,0,"                ");//
                                lcd1602_cursor(1,0);
								now_window = inputpassword;
                            }
                            break;
                            case 11://��������
                            {
                                wri_string(1,0,"Put Old Password ");//
	                            wri_string(2,0,"                 ");//
	                            lcd1602_cursor(1,0);
								now_window = changeoldpassword;
                            }
                            break;
                            case 12://��������
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
              case inputpassword://��������
              {
                        switch (key_value)
                        {
                        			case 1: case 2:case 3:
                        			case 4: case 5:case 6:
                        			case 7: case 8:case 9:case 0:
                        			{
                        				password_now[curr_bit] = key_value;
                        				if (++curr_bit>6)//����6λ����
                        				{
                        					curr_bit = 6;
                        				}
                        				curr_bit_tmp = curr_bit;
                        				for(i=0;i<curr_bit_tmp;i++)//��ʾ*��
                        				dis_dat[i] = '*';
										wri_string(2,0,dis_dat);
										lcd1602_cursor(1, curr_bit);
                        			}
        			                break;
        			                case 14://����#ɾ������
                        			{
                        				password_now[curr_bit] = 20;
                        				if (--curr_bit<0)
                        					curr_bit = 0;
										dis_dat[curr_bit] = ' ';//��ʾΪ��
										wri_string(2,0,dis_dat);
										lcd1602_cursor(1, curr_bit);
                        			}
        			                break;
                			        case 15://����ȷ�ϱȶ�����
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

                        					relay = 0;//�򿪼̵���
                        					wri_string(1,0,"    success    ");
											wri_string(2,0,"     open      ");
											lcd1602_cursor(0, curr_bit);
											now_window = keynoser;//��������Ӧ
                        				}
                        				else
                        				{
											  password_err_cnt++;
                        					  open_flag = 1;//�����־
                        					  wri_string(1,0,"password err   ");
											  wri_string(2,0,"  err cnt:     ");
											  lcd_write_char(2,10,password_err_cnt);
											  lcd1602_cursor(0, curr_bit);
											  now_window = keynoser;//��������Ӧ
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
              case changeoldpassword://�޸���ǰ���������
              {
                        switch (key_value)
                        {
                        			case 1: case 2:case 3:
                        			case 4: case 5:case 6:
                        			case 7: case 8:case 9:case 0:
                        			{
                        				password_now[curr_bit] = key_value;
                        				if (++curr_bit>6)//����6λ����
                        				{
                        					curr_bit = 6;
                        				}
                        				curr_bit_tmp = curr_bit;
                        				for(i=0;i<curr_bit_tmp;i++)//��ʾ*��
                        				dis_dat[i] = '*';
										wri_string(2,0,dis_dat);
										lcd1602_cursor(1, curr_bit);
                        			}
        			                break;
        			                case 14://����#ɾ������
                        			{
                        				password_now[curr_bit] = 20;
                        				if (--curr_bit<0)
                        					curr_bit = 0;
										dis_dat[curr_bit] = ' ';//��ʾΪ��
										wri_string(2,0,dis_dat);
										lcd1602_cursor(1, curr_bit);
                        			}
        			                break;
                			        case 15://����ȷ�ϱȶ�����
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
											now_window = changepassword;//��������Ӧ
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
                                              wri_string(1,0,"  welcome user  ");//��ʼ����ʾ
					                          wri_string(2,0,"A:open  B:modify");//��ʼ����ʾ
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
              case changepassword://�޸�����
              {
					 switch (key_value)
					 {
							case 1: case 2:case 3:
                			case 4: case 5:case 6:
                			case 7: case 8:case 9:case 0:
                			{
                				password_now[curr_bit] = key_value;
                				if (++curr_bit>6)//����6λ����
                				{
                					curr_bit = 6;
                				}
                				curr_bit_tmp = curr_bit;
                				for(i=0;i<curr_bit_tmp;i++)//��ʾ*��
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
								dis_dat[curr_bit] = ' ';//��ʾΪ��
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
								wri_string(1,0,"   put again    ");//�ٴ�����
	                            wri_string(2,0,"                ");//
	                            lcd1602_cursor(1,0);
								now_window = putagain;
							}
							break;
					 }
              }
              break;
			  case putagain://�޸�������ٴ�����
			  {
			  	 switch (key_value)
			  	 {
				 			case 1: case 2:case 3:
                			case 4: case 5:case 6:
                			case 7: case 8:case 9:case 0:
                			{
                				chang_pass[curr_bit] = key_value;
                				if (++curr_bit>6)//����6λ����
                				{
                					curr_bit = 6;
                				}
                				curr_bit_tmp = curr_bit;
                				for(i=0;i<curr_bit_tmp;i++)//��ʾ*��
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
								dis_dat[curr_bit] = ' ';//��ʾΪ��
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
										password[i] = chang_pass[i];   //��������
									}
									At24c02WriteBuf(password,0,6);
									delay_ms(300);
									beep = 0;
									delay_ms(2000);
									beep = 1;
									now_window = chosemod;
									wri_string(1,0,"  welcome user  ");//��ʼ����ʾ
									wri_string(2,0,"A:open  B:modify");//��ʼ����ʾ
								}
								else
								{
									    cha_err_flag = 1;
										wri_string(1,0," Two different  ");
										wri_string(2,0,"     return     ");
										now_window = keynoser;//��������Ӧ
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
 ** ��������init_all_hardware
 ** ����  ����ʼ������Ӳ�����������������
 ** ����  ����
 ** ���  ����
 ** ����  ��������
 ******************************************************/
void init_all_hardware(void)
{
   		delay_ms(100);
		time_init();//��ʱ����ʼ��
        lcd_init();//Һ����ʼ��
		At24c02init();//�洢��ʼ��
		key_value = 20;//Ĭ��û��������
		now_window = chosemod;//ѡ��ģʽ
		cnt_100ms = 0;
		time_100ms_flag = 0;
		open_del = 0;
		relay = 1;//�̵����رա�
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
	 init_all_hardware();//��ʼ��Ӳ����IO�Ͷ�ʱ��
	 while(1)
	 {
	 	 key_scan();//����ɨ��
		 key_service();//������������
		 time_service();//ʱ�䴦����
	 }
}
 /******************************************************
 ** ��������time0_interrupt
 ** ����  ������ɨ�躯��
 ** ����  ����
 ** ���  ����
 ******************************************************/
void time0_interrupt() interrupt 1
{
	   TF0 = 0;//�����־
	   TR0 = 0;
	   if (++cnt_100ms>50)
	   {
			cnt_100ms = 0;
			time_100ms_flag = 1;
	   }
	   TR0 = 1;
	   TH0 = 0xf8;
	   TL0 = 0x2f;//װ�س�ֵ2ms(65535-63535)
}
