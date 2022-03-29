int read_txt()
{
	ifstream infile;                                                                     //定义读取文件流，相对于程序来说是in
	infile.open("E://file.txt");         //打开文件
	for (int i = 0; i < 65536; i++)                                                      //定义行循环
	{
		infile >> data_all[i];                                                             //读取一个值（空格、制表符、换行隔开）就写入到矩阵中，行列不断循环进行
	}
	infile.close();                                                                      //读取完成之后关闭文件
	return 0;
}
