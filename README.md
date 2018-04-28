
1
===
2
-----
3
TLC320ADC3101 Low-Power Stereo ADC With Embedded miniDSP for Wireless Handsets and Portable Audio
SDRAM和处理器寻址的几点理解


每个芯片的是16跟数据线，x2就组成32位数据线。地址地址13根，BA0 BA1 接到ADDR24 ADDR25   【这个是片选择哪一个bank】，为什么？
http://blog.csdn.net/zmnqazqaz/article/details/51298289
http://blog.csdn.net/kickxxx/article/details/7231621
http://blog.csdn.net/blueice8601/article/details/7377092

1，	先弄清楚第一个概念，对于CPU来说:
 一个地址用32位来表示：0x30000000、0x40000000; 但是这个地址代表的地址空间是1个字节，也就是说CPU读取0x30000000这个地址得到的是一个字节的数据； 一个地址是多少位的地址跟这个地址代表的地址空间有多大是2码事。
CPU：
          0x30000000         1字节
          0x30000001         1字节
          0x30000002         1字节
2，
  ① 对于JZ2440来说，SDRAM有2片，每一片的数据输出宽度是16bit，总共构成了具有32bit数据输出的内存，现在这块内存1次输出32bit的数据。
  ② 在这种情况下，读取SDRAM的一个地址得到的数据是4个字节
现在就出现了一个矛盾：  对于CPU来说一个地址代表1个字节的数据，但是我读一次SDRAM，它会给我4字节的数据，怎么办？ 

假设：下图黑色带序号的表示CPU想得到的数据，①②③④⑤...数据大小都是1个字节


0x30000000是SDRAM的首地址，现在CPU想得到①开始到⑨的9个字节数据。①②③...⑨这9个字节在SDRAM实际是怎么存储的呢？ 

 也就是说①②③④这四个字节的数据在SDRAM的0地址，⑤⑥⑦⑧这四个字节的数据在SDRAM的1地址，⑨这个字节在SDRAM的2地址。     
那么，     
 ⑴   CPU发出0x30000000、0x30000001、0x30000002、0x30000003这4个地址的时候，访问的都必须是SDRAM的0地址
 ⑵   发出0x30000004、0x30000005、0x30000006、0x30000007这4个地址的时候，访问的都必须是SDRAM的1地址
⑶   发出0x30000008这个地址的时候，访问的必须是SDRAM的2地址。       而CPU的地址线是直接连在SDRAM的地址线上的，中间又没有其他硬件或者软件做转换，要做到上面这3点，怎么办？       我们把这个地址对应关系写的更简单一些：   


 0x30000000、0x30000001....这些地址是32位的，每一位对应一根地址线（这句话很重要）。       如果我们把上面的地址对应表以二进制的形式表示出来会更加直观：



最左边的数字是地址的10进制表示，中间是这个地址代表的二进制，最右边是对应的SDRAM的地址。
CPU要想发出地址后得到正确数据，那么发出的地址的bit0、bit1是无效的，地址从bit2开始有效。
实际有效地址的最低位是从bit2开始的：     
  无论bit0、bit1的值是多少，bit2为0的时候访问的是SDRAM的0地址；                             
                 bit2为1的时候访问的是SDRAM的1地址；                             
                 bit3、bit2为10的时候访问的是SDRAM的2地址；                               
                ......     
 既然实际有效地址的最低位从bit2开始，那么把CPU的ADDR2地址线接到SDRAM的AD0;                                                           
          把CPU的ADDR3地址线接到SDRAM的AD1;                                                                    
 把CPU的ADDR4地址线接到SDRAM的AD2;                                                                 
        .......     
 CPU的ADDR0和ADDR1不用接。  

实这个也只跟位宽有关系。
总结一下：CPU一定是字节寻址（8bit）
                内存芯片的寻址是看数据位宽
            
               我们把统一单位的地址线连接起来就可以了。

               比如A0是2的0次方字节    1字节
                      A1是2的1次方字节    2字节
                      A2是2的2次方字节    4字节

               那么位宽8位  就是1字节  从A0开始连接
                          16位  就是2字节  从A1开始连接

               以此类推
                    
我们看一下SDRAM的原理 就是一个立方体， 行x列x几（bank）        

寻址就非常简单，首先CPU发送第几个bank信号，对应ba0 ba1的值，然后通过地址依次发 行   列  信号，怎么知道是行还是列就看 nSRAS  nSCAS 行/列 有效管教是不是被置位了
一片DRAM其实是由4个4Mx16的存储块也就是bank组成，其中4M指的是4Mbit个存储单元，16指的是每个存储单元存储的位数，即2个字节。需要知道的是这4片存储块是相互独立的，并且每个bank都是阵列结构，所谓阵列结构指的是它是由行列组成的。一个bank看做是由许多方格组成的存储块，而方格的个数就是4Mbit,每个方格的容量则为16bit，你要想确定一个方格的地址首先要确定它的行地址，再确定它的列地址，两线相交确定一点，这一点就是你要的地址。一般SDRAM的地址线都采用分时复用原则，即先送行地址，再送列地址，K4S561632C这款芯片Row address : RA0 ~ RA12, Column address : CA 0 ~ CA8,即前9个地址线是被行列复用的。所以寻址的时候先发送bank地址和RA0 ~ RA12行地址，然后再发送CA 0 ~ CA8列地址。按照行地址13个，列地址9个，总容量为4 * 2^13 * 2 ^ 9 * (16 / 8) = 32MB
3、为什么bank0和bank1与2440的ADDR24和ADDR25连接?
我们已经了解到bank0和bank1是用来选择存储块 (bank)的，32M的空间需要25根地址线寻址,但是对于K4S561632C只需要24根,因为它每个存储单元存储2个字节,我们来分一下,这24根地址线的前13根行地址,紧接着的9根为列地址,那么最后两根是干嘛？显而易见它是bank地址,用来选择存储块,所以BANK0接23, BANK1接24,但是由于我们是从ADDR2开始连得原因前面已经解释过了,所以每个地址加2.最后的地址连接方法,就是ADDR2~ ADDR14对应行地址, ADDR15~ ADDR23对应列地址,(由于管脚的分时复用其实是DDR2~ ADDR14对应行地址, ADDR2~ ADDR10对应列地址)ADDR24~ ADDR25对应BANK0~ BANK1,这就是bank0和bank1与2440的ADDR24和ADDR25连接的原因。

Bank Size:外接内存容量大小（4Mbit*16bit*4Bank*2Chips/8=64MB）
Bus Width:总线宽度 （两片16位HY57561620，并联成32位）
Base Component：单个芯片容量(bit)（256Mb）
Memory Configration：内存配置 (（4M*16*4banks）*2Chips )

4、由两片K4S561632C组合在一起的,这样做的效果是什么？
我们已经知道1片K4S561632C有4个bank,两片K4S561632C就有8个bank了,当连接一个K4S561632C时,一个地址对应2个字节,现在连接了两片K4S561632C那么,一个地址就对应4个字节了,即一个字,总共64MB,当一个地址存放1个字节,我们需要64Mbit个地址才能达到64MB的容量,现在容量不变,每个地址里存放的字节数增多了,那么必然需要的地址数就减少了,根据存储容量（BYTE）=地址数*每个地址能存的字节数,可知当每个地址里存放的字节数增大4倍时,我们的地址数会变为原来的1/4, 当一个地址存放4个字节,我们只需要16Mbit个地址就能达到64MB的容量,16Mbit代表24根地址线(2^24=16Mbit=16*1024*1024)
MEMCONTROL里面有个num_chip，使用了几个CS信号。两个DDR控制器只用了一个CS0，对于210来说就是一片，所以设置为 0x0 = 1 chip
然后 MEMCONTROL的mem_width，设为 0x2 = 32-bit 。
MEMCONFIG0和MEMCONFIG1是一样的，一个设置相应控制器的CS0，一个设置CS1。
 
这两个寄存器合起来，可以决定DMC0/DMC1 CS0和CS1下挂着的内存对应哪一段内存地址。我理解的是当你要放问地址0x21xx_xxxx的地址的时候，DMC0会将访问地址的高8bits 0x21和chip_mask求与，要是等于chip_base，他就会使用相应的控制器的相应的片选去读数据。就像假如
DMC0_MEMCONFIG0   chip_base =  0x20      chip_mask=0xf8
DMC0_MEMCONFIG1   chip_base =  0x28      chip_mask=0xf8
当你要访问的地址是 0x22xx_xxxx的时候，  （0x22 & 0xf8） ==  0x20，所以使用CS0
那要是  0x2exx_xxxx呢？     （0x2e & 0xf8） == 0x28 ，当然使用CS1了。
也就是说CS0对应的是 0x2000_0000 ~ 0x27ff_ffff，128M
CS1对应的是 0x2800_0000 ~ 0x28ff_ffff，128M
同样的每个CS下都是256M呢？
DMC0_MEMCONFIG0   chip_base =  0x20      chip_mask=0xf0          // 0x2000_0000 ~ 0x2fff_ffff     256M
DMC0_MEMCONFIG1   chip_base =  0x30      chip_mask=0xf0          // 0x3000_0000 ~ 0x3fff_ffff     256M
DMC0和DMC1配置一样，但是DMC0只能配置为0x2000_0000~0x3fff_ffff的空间，DMC1只能配置为0x4000_0000~0x5fff_ffff的空间。这是DMC的地址空间决定的，我就是在这郁闷了两天。
我的板子上面DMC0和DMC1上都只使用了CS0,然后我的设置就是
DMC0_MEMCONFIG0   chip_base =  0x20      chip_mask=0xf0          // 0x2000_0000 ~ 0x2fff_ffff     256M
DMC1_MEMCONFIG0   chip_base =  0x40      chip_mask=0xf0          // 0x4000_0000 ~ 0x4fff_ffff     256M  注意，DMC1只能从0x4000_0000开始。
问题又来了，这样内存地址空间不就不连续了嘛？
内存的地址空间不从0x2000_0000开始，从0x3000_0000开始，不就正好接上DMC1的了。只是CONFIG_SYS_SDRAM_BASE和CONFIG_SYS_TEXT_BASE相应的改一下：
DMC0_MEMCONFIG0   chip_base =  0x30      chip_mask=0xf0          // 0x3000_0000 ~ 0x3fff_ffff     256M
DMC1_MEMCONFIG0   chip_base =  0x40      chip_mask=0xf0          // 0x4000_0000 ~ 0x4fff_ffff     256M
 
难点：关于地址映射
如果设置chip_base为0x20：
(1)我们挂载的内存为128M，那么这个chip_mask应该设置为0xF8
(2)我们挂载256M内存时，chip_mask应该设置为0xF0
(3)我们挂载512M时，chip_mask应该设置为0xE0
(4)我们挂载1GB内存时，chip_mask就应该设置为0xC0。
以DMC0为例，当DMC0接收到来自AXI的0x2000,0000~0x3fff,ffff内的地址时，会作如下处理：
(1)将AXI地址的高8位与chip_mask相与得到结果，记为X。
(2)将X分别与MEMCONFIG0和MEMCONFIG1的chip_base相比较，如果相等，则打开相应的片选。
假如挂载的内存为128M，且CS0和CS1上分别挂了一片，那么128M=128*1024*1024=0x8000000，则128M内存的偏移范围应该是0x0000,0000~0x07ff,ffff，高位剩余5位，那么，我们把MEMCONFIG0的chip_base设置为0x20，chip_mask设置为0xF8，为了保持内存连续，则需要将MEMCONFIG1的chip_base设置为0x28，chip_mask设置为0xF8，当AXI发来的地址为0x23xx,xxxx时，0x23&0xF8得到0x20，所以，会打开片选CS0，当AXI发来的地址为0x28xx,xxxx时，0x28&0xF8得到0x28，所以，会打开片选CS1，依此类推。
特别的，当载在的内存芯片为8bank（8bank内存芯片一般为14/15行地址，10列地址，即容量一般为512M或者1G）时，由于CS1为bank2引脚，为了保持CS0时钟处于片选状态，对于512M内存来讲需要将chip_mask设置为0xE0，这是因为512M=512*1024*1024=0x2000,0000，也就是说，512M内存的偏移应该为0x0000,0000~0x1fff,ffff，所以高位剩余3位，即0xE0，当然了，如果内存为1G=1024*1024*1024=0x4000,0000，即偏移为0x0000,0000~0x3fff,ffff，高位剩余2为，故设置chip_mask为0xC0。这样，就会计算偏移这两个值了。
http://blog.chinaunix.net/uid-122754-id-3144920.html
http://www.cnblogs.com/Efronc/archive/2012/03/01/2375578.html

