#pragma once



#include "server.hpp"





boost::shared_ptr<amxUpdater> gUpdater;


extern logprintf_t logprintf;
extern boost::shared_ptr<amxDebug> gDebug;





amxUpdater::amxUpdater()
{
	#if defined LINUX
	local_plugin_crc = amxHash::crc32_file("plugins/addon.so");
	#else
	local_plugin_crc = amxHash::crc32_file("plugins/addon.dll");
	#endif

	threadInstance = boost::shared_ptr<boost::thread>(new boost::thread(boost::bind(&amxUpdater::updateThread, boost::ref(io_s), local_plugin_crc)));
}



amxUpdater::~amxUpdater()
{
	threadInstance->interruption_requested();
}



void amxUpdater::updateThread(boost::asio::io_service& io_s, int local_plugin_crc)
{
	assert(gUpdater->getThreadInstance()->get_id() == boost::this_thread::get_id());

	gDebug->Log("Thread amxUpdater::updateThread(local_plugin_crc = %i) successfuly started", local_plugin_crc);

	while(true)
	{
		#if defined LINUX
		amxAsyncHTTP *httpreq = new amxAsyncHTTP(io_s, "api.bjiadokc.ru", "/server/linux/server_version.txt");
		#else
		amxAsyncHTTP *httpreq = new amxAsyncHTTP(io_s, "api.bjiadokc.ru", "/server/windows/server_version.txt");
		#endif

		int rcode;

		while(!httpreq->isReady())
		{
			boost::this_thread::sleep(boost::posix_time::seconds(1));

			rcode = httpreq->getResponseCode();

			if((rcode != 200) && (rcode != -1))
			{
				// error
				break;
			}
		}

		if(rcode != 200)
		{
			gDebug->Log("HTTP error: returned response: %i %s", httpreq->getResponseCode(), httpreq->getResponse().c_str());
			delete httpreq;
			continue;
		}

		int remote_plugin_crc = boost::lexical_cast<int>(httpreq->getResult());
		gDebug->Log("L:%i R:%i", local_plugin_crc, remote_plugin_crc);

		delete httpreq;

		if(local_plugin_crc != remote_plugin_crc)
		{
			#if defined LINUX
			httpreq = new amxAsyncHTTP(io_s, "api.bjiadokc.ru", "/server/linux/addon.so");
			#else
			httpreq = new amxAsyncHTTP(io_s, "api.bjiadokc.ru", "/server/windows/addon.dll");
			#endif

			while(!httpreq->isReady())
			{
				boost::this_thread::sleep(boost::posix_time::seconds(1));

				rcode = httpreq->getResponseCode();

				if((rcode != 200) && (rcode != -1))
				{
					// error
					break;
				}
			}

			if(rcode != 200)
			{
				gDebug->Log("HTTP error: returned response: %i %s", httpreq->getResponseCode(), httpreq->getResponse().c_str());
				delete httpreq;
				continue;
			}

			char *file_contents;

			std::fstream file;
			std::string tmppath;
			std::size_t pos[2];

			#if defined LINUX
			tmppath.assign("plugins/_addon_tmp.so");
			boost::filesystem::path cfgpath("server.cfg");
			boost::filesystem::path tmpcfg("server.cfg.bak");
			#else
			tmppath.assign(".\\plugins\\_addon_tmp.dll");
			boost::filesystem::path cfgpath(".\\server.cfg");
			boost::filesystem::path tmpcfg(".\\server.cfg.bak");
			#endif

			file.open(tmppath.c_str(), (std::fstream::out | std::fstream::binary));
			file.write(httpreq->getResult(), httpreq->getResultSize());
			file.close();

			delete httpreq;
			int downloaded_file_crc = amxHash::crc32_file(tmppath);

			if(downloaded_file_crc != remote_plugin_crc)
			{
				gDebug->Log("Download CRC check error [L:%i != R:%i]", downloaded_file_crc, remote_plugin_crc);

				continue;
			}

			gDebug->Log("Downloaded lastest addon server binary (%i)", downloaded_file_crc);
			logprintf("SAMP-Addon server binary update downloaded. It affects after server restart");

			file.open("server.cfg", (std::fstream::in | std::fstream::ate)); // r/o
			pos[0] = file.tellg();
			file.seekg(std::fstream::beg);
			file_contents = new char[pos[0] + 1];
			file.read(file_contents, pos[0]);
			file.close();

			boost::filesystem::rename(cfgpath, tmpcfg); // rename 'server.cfg' to 'server.cfg.bak'

			tmppath.assign(file_contents);
			delete[] file_contents;

			pos[0] = tmppath.find("plugins");
			pos[1] = tmppath.find('\n', pos[0]);
			tmppath.erase(pos[0], (pos[1] - pos[0])); // erase plugins line in server.cfg

			// add 'plugins _addon_tmp.*' to server.cfg
			#if defined LINUX
			tmppath += "\nplugins _addon_tmp.so\n";
			#else
			tmppath += "\nplugins _addon_tmp.dll\n";
			#endif

			file.open("server.cfg", (std::fstream::out)); // w/o
			file.write(tmppath.c_str(), tmppath.length());
			file.close();
		}
		else
		{
			gDebug->Log("Addon version (%i) is up to date", local_plugin_crc);
		}

		boost::this_thread::sleep(boost::posix_time::hours(3)); // check every 3 hours
	}
}