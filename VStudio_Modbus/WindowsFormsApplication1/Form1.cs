using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace WindowsFormsApplication1
{
    public partial class Form1 : Form
    {
        public string SValor { get; set; }
        public int Voltage, Temperature, InputPJ1;
        public int rcnt;
        public int rec_step;
        public byte[] rec = { 0, 0, 0, 0, 0, 0, 0, 0 };
        public int reqAddr = 0;

        public Form1()
        {
            InitializeComponent();
        }

        static byte[] GetBytes(string str)
        {
            byte[] bytes = new byte[str.Length * sizeof(char)];
            System.Buffer.BlockCopy(str.ToCharArray(), 0, bytes, 0, bytes.Length);
            return bytes;
        }

        private void DataReceivedHandler(object sender,
         System.IO.Ports.SerialDataReceivedEventArgs e)
        {
            byte[] data = new byte[serialPort1.BytesToRead];
            serialPort1.Read(data, 0, data.Length);

            if ((data[0] == 0x05) && (data.Length>6))
            {
                if (reqAddr == 0)
                {
                    Voltage = (int)data[3] * 256 + data[4];
                }
                else if (reqAddr == 2)
                {
                    Temperature = (int)data[3] * 256 + data[4];
                }
                else if (reqAddr == 4)
                {
                    InputPJ1 = (int)data[3] * 256 + data[4];
                }
            } 
   }
   
        private void textBox1_KeyPress(object sender, KeyPressEventArgs e)
        {
            if (char.IsNumber(e.KeyChar) || (e.KeyChar == '.') || (e.KeyChar == ','))
            {

            }
            else
            {
                e.Handled = e.KeyChar != (char)Keys.Back;
            }
        }

        private void timer_Tick(object sender, EventArgs e)
        {
            float FVoltage;                             // auxiliar para visualizar casas decimais
            reqAddr = (reqAddr == 4) ? 0 : reqAddr + 2;     // endereco da variavel a ser lida : 0 , 2 e 4;
            request();                                  // send request
            FVoltage = (float)Voltage / 1000;       
            setTemp.Text = FVoltage.ToString();
            ATemp.Text = Temperature.ToString();
            PJ1.Text = InputPJ1.ToString();
        }


        private void Form1_Load(object sender, EventArgs e)
        {

        }

        private void button2_Click(object sender, EventArgs e)
        {
            if (!serialPort1.IsOpen)
            {
                serialPort1.Open();
            }

            timer.Enabled = true;
            timer.Start();
        }

        private void request()
        {
            byte[] envia = { 0, 0, 0, 0, 0, 0, 0, 0 };
            int crcValue;
            int crc0, crc1;

            envia[0] = 05;                      // endereco Modbus
            envia[1] = 03;                      // le parametro
            envia[2] = 00;                      // endereco 
            envia[3] = (byte)reqAddr;           // endereco
            envia[4] = 00;                      // quantidade
            envia[5] = 01;                      // quantidade
            crcValue = crc(envia, 0, 6);
            crc0 = (crcValue & 0xFF00) >> 8;
            envia[6] = (byte)crc0;              // CRC
            crc1 = (crcValue & 0x00FF);
            envia[7] = (byte)crc1;              // CRC

            serialPort1.Write(envia, 0, 8);
            
        }

        private int crc(byte[] buf, int start, int cnt) 
        {
            int i, j;
            int temp, temp2;
            int flag;

            temp = 0xFFFF;

            for (i = start; i < cnt; i++)
            {
                temp = temp ^ buf[i];

                 for (j = 1; j <= 8; j++)
                 {
                    flag = temp & 0x0001;
                    temp = temp >> 1;
                    if (flag==1)
                    {
                        temp = temp ^ 0xA001;
                    }
                 }
           }
        

            /* Reverse byte order. */
            temp2 = temp >> 8;
            temp = (temp << 8) | temp2;
            temp &= 0xFFFF;

            return (temp);
        }

    }
}
