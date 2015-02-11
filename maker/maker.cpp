#pragma once



#include "maker.hpp"





int crc32_file(std::string filename)
{
	boost::crc_32_type result;
	std::ifstream i;

	i.open(filename, std::fstream::binary);
		
	do
	{
		char block[2048];
		
		i.read(block, sizeof block);
		result.process_bytes(block, i.gcount());
	}
	while(i);

	i.close();

	return result.checksum();
}



int main()
{
	SetConsoleCP(1251);
	SetConsoleOutputCP(1251);

	std::string cmdline(GetCommandLine());

	if(cmdline.find("/crcgen") != std::string::npos)
	{
		std::cout << "CRC generation process started..." << std::endl;

		boost::filesystem::path version_af("..\\build\\client\\client_version.txt");
		boost::filesystem::path version_uf("..\\build\\client\\updater_version.txt");
		boost::filesystem::path version_wsf("..\\build\\server\\windows\\server_version.txt");
		boost::filesystem::path version_lsf("..\\build\\server\\linux\\server_version.txt");

		std::ofstream f;
		int fcrc;

		if(boost::filesystem::exists(version_af))
			boost::filesystem::remove(version_af);

		if(boost::filesystem::exists(version_uf))
			boost::filesystem::remove(version_uf);

		if(boost::filesystem::exists(version_wsf))
			boost::filesystem::remove(version_wsf);

		if(boost::filesystem::exists(version_lsf))
			boost::filesystem::remove(version_lsf);
		
		fcrc = crc32_file("..\\build\\client\\d3d9.dll");
		f.open("..\\build\\client\\client_version.txt", std::ofstream::out);
		f << fcrc;
		f.close();
		std::cout << "d3d9.dll (client) processed (CRC: " << fcrc << ")" << std::endl;

		fcrc = crc32_file("..\\build\\client\\updater.exe");
		f.open("..\\build\\client\\updater_version.txt", std::ofstream::out);
		f << fcrc;
		f.close();
		std::cout << "updater.exe (client) processed (CRC: " << fcrc << ")" << std::endl;

		fcrc = crc32_file("..\\build\\server\\windows\\addon.dll");
		f.open("..\\build\\server\\windows\\server_version.txt", std::ofstream::out);
		f << fcrc;
		f.close();
		std::cout << "addon.dll (server) processed (CRC: " << fcrc << ")" << std::endl;

		fcrc = crc32_file("..\\build\\server\\linux\\addon.so");
		f.open("..\\build\\server\\linux\\server_version.txt", std::ofstream::out);
		f << fcrc;
		f.close();
		std::cout << "addon.so (server) processed (CRC: " << fcrc << ")" << std::endl;

		std::cout << "CRC generation process finished" << std::endl;
	}

	if(cmdline.find("/cleanup") != std::string::npos)
	{
		std::cout << "Cleanup process started" << std::endl;

		boost::filesystem::path cliexp("..\\build\\client\\d3d9.exp");
		boost::filesystem::path clilib("..\\build\\client\\d3d9.lib");
		boost::filesystem::path clipdb("..\\build\\client\\d3d9.pdb");
		boost::filesystem::path sexp("..\\build\\server\\windows\\addon.exp");
		boost::filesystem::path slib("..\\build\\server\\windows\\addon.lib");
		boost::filesystem::path spdb("..\\build\\server\\windows\\addon.pdb");
		boost::filesystem::path uppdb("..\\build\\client\\updater.pdb");
		boost::filesystem::path ipch("..\\ipch");
		boost::filesystem::path clR("..\\client\\Release");
		boost::filesystem::path svR("..\\server\\Release");
		boost::filesystem::path mkR("..\\maker\\Release");
		boost::filesystem::path upR("..\\updater\\Release");

		try
		{
			boost::filesystem::remove(cliexp);
			boost::filesystem::remove(clilib);
			boost::filesystem::remove(clipdb);
			boost::filesystem::remove(sexp);
			boost::filesystem::remove(slib);
			boost::filesystem::remove(spdb);
			boost::filesystem::remove(uppdb);
			boost::filesystem::remove_all(ipch);
			boost::filesystem::remove_all(clR);
			boost::filesystem::remove_all(svR);
			boost::filesystem::remove_all(mkR);
			boost::filesystem::remove_all(upR);
		}
		catch(boost::filesystem::filesystem_error &err)
		{
			std::cout << "Error while cleaning up (What: " << err.what() << ")" << std::endl;
		}

		std::cout << "Cleanup process finished" << std::endl;
	}

	return 0;
}