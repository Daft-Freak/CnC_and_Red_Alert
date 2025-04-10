#include <algorithm>
#include <fstream>
#include <list>
#include <set>
#include <string>
#include <unordered_map>

#ifdef _WIN32
#include <direct.h>
#else
#include <sys/stat.h>
#endif

#include "base64.h"
#include "ccfile.h"
#include "crc.h"
#include "mixfile.h"
#include "rndstraw.h"

#include "mixwriter.h"
#include "names.h"

using CCMixFile = MixFileClass<CCFileClass>;

static std::unordered_map<int32_t, const char *> name_map;

// game load order, first one takes priority
static const char *mix_priority[]
{
	// Bootstrap
	"WOLAPI.MIX",
	"EXPAND2.MIX",
	"HIRES1.MIX", // only one of
	"LORES1.MIX", // these is loaded
	"EXPAND.MIX",
	"REDALERT.MIX", // container
	"LOCAL.MIX",
	"HIRES.MIX", // only one of
	"LORES.MIX", // these is loaded
	"NCHIRES.MIX", // hires

	// Expansion
	// SC*/SS*

	// Secondary
	"MAIN.MIX", // container
	"CONQUER.MIX",
	"GENERAL.MIX",
	"MOVIES1.MIX",
	"MOVIES2.MIX", // only if MOVIES1 isn't found
	"SCORES.MIX",
	"SPEECH.MIX",
	"SOUNDS.MIX",
	"RUSSIAN.MIX",
	"ALLIES.MIX",
};

// files only used by hi/lores, but not in hi/lores.mix
static std::set<std::string> hires_only
{
	// score screen
	"ALIBACKH.PCX",
	"BAR3BHR.SHP",
	"BAR3RHR.SHP",
	"CREDSAHR.SHP",
	"CREDSUHR.SHP",
	"HISC1-HR.SHP",
	"HISC2-HR.SHP",
	"SOVBACKH.PCX",
	"TIMEHR.SHP",	
};

static std::set<std::string> lores_only
{
	// score screen
	"ALI-TRAN.WSA",
	"BAR3BLU.SHP",
	"BAR3RED.SHP",
	"CREDSA.SHP",
	"CREDSU.SHP",
	"HISCORE1.SHP",
	"HISCORE2.SHP",	
	"SOV-TRAN.WSA",
	"TIME.SHP",
};

static PKey *key;

// list state
static MixFileClass<CCFileClass> *last_mix = nullptr;

// keep track of all mix files so we can clean them up
// (because we unlinked the list)
std::set<CCMixFile *> mix_files;

// keep track of nesting
static std::unordered_map<CCMixFile *, std::tuple<CCMixFile *, int32_t>> mix_parents;

struct FileEntry
{
	const char *name;
	CCMixFile *mix;
	CCMixFile::SubBlock *block;
};

static std::unordered_multimap<int32_t, FileEntry> all_files;

static bool create_directory(const char *path) {
#ifdef _WIN32
  	return _mkdir(path) == 0 || errno == EEXIST;
#else
  	return mkdir(path, 0755) == 0 || errno == EEXIST;
#endif
}

static const char *get_filename(int32_t crc)
{
    auto it = name_map.find(crc);
    if(it != name_map.end())
        return it->second;
	return nullptr;
}

static void mix_file_callback(CCMixFile *mix, CCMixFile::SubBlock *entry)
{
    if(mix != last_mix)
    {
		// unlink the files as we go so we don't find a file in the old one
		// if we're trying to overlay multiple files
		if(last_mix)
			last_mix->Unlink();

        last_mix = mix;

		auto parent_it = mix_parents.find(mix);
		if(parent_it != mix_parents.end())
			printf("\n%s (%s):\n", mix->Filename, std::get<0>(parent_it->second)->Filename);
		else
        	printf("\n%s:\n", mix->Filename);
    }

	mix_files.insert(mix);

    auto filename = get_filename(entry->CRC);

    if(filename)
    {
        auto ext = strrchr(filename, '.');

        if(ext)
        {
            // register nested mixfile
            if(strcmp(ext, ".MIX") == 0)
			{
                auto new_mix = new CCMixFile(filename, key);
				mix_parents.emplace(new_mix, std::make_tuple(mix, entry->Offset));
			}
		}
        printf("%-12s size %9u offset %9u\n", filename, entry->Size, entry->Offset);
    }
    else
        printf("UNK %08X size %9u offset %9u\n", entry->CRC, entry->Size, entry->Offset);

	// add to list
	FileEntry file{filename, mix, entry};

	all_files.emplace(entry->CRC, file);
}

int main(int argc, char *argv[])
{
    // init name map
    for(int i = 0; i < num_filenames; i++)
    {
		auto name = filenames[i];
        auto crc = CRCEngine()(name, strlen(name));
        name_map.emplace(crc, name);
    }

	// filter config
	bool hires = false;
	bool editor = false;

    // init key
    char exp_buf[512];
    BigInt exp = PKey::Fast_Exponent();
    exp.DEREncode((unsigned char *)exp_buf);

    char mod_buf[42];
    Base64_Decode("AihRvNoIbTn85FZRYNZRcT+i6KpU+maCsEqr3Q5q+LDB5tH7Tz2qQ38V", 56, mod_buf, 42);
    key = new PKey(exp_buf, mod_buf);

    // register files
    for(int i = 1; i < argc; i++)
	{
		if(argv[i][0] == '-')
		{
			if(strcmp(argv[i], "--hires") == 0)
				hires = true;
			if(strcmp(argv[i], "--editor") == 0)
				editor = true;
		}
		else
        	new CCMixFile(argv[i], key);
	}

    // list them
    CCMixFile::List_All(mix_file_callback);

	printf("\nfiltering...\n");

	// filter out unwanted files (hi/lores, editor, etc.)
	for(auto it = all_files.begin(); it != all_files.end();)
	{
		bool remove = false;

		auto mix_filename = it->second.mix->Filename;

		// LORES, LORES1, EDLO
		if(hires && (strstr(mix_filename, "LORES") || strcmp(mix_filename, "EDLO.MIX") == 0))
			remove = true;
		// HIRES, HIRES1, NCHIRES, EDHI
		else if(!hires && (strstr(mix_filename, "HIRES") || strcmp(mix_filename, "EDHI.MIX") == 0))
			remove = true;

		if(!editor && (strcmp(mix_filename, "EDITOR.MIX") == 0 || strcmp(mix_filename, "EDLO.MIX") == 0 || strcmp(mix_filename, "EDHI.MIX") == 0))
			remove = true;

		// additional lo/hires-only files
		if(hires && it->second.name && lores_only.count(it->second.name))
			remove = true;
		else if(!hires && it->second.name && hires_only.count(it->second.name))
			remove = true;

		// video and palette table for a large video not referenced by the game
		if(it->first == 0x18FF53F1 || it->first == 0x27FF53F1)
			remove = true;

		if(remove)
		{
			printf("removing %s in %s\n", it->second.name, mix_filename);
			it = all_files.erase(it);
		}
		else
			++it;
	}

	// deduplication
	for(auto it = all_files.begin(); it != all_files.end();)
	{
		// find next entry
		auto next_key = std::next(it);
		int count = 1;
		while(next_key != all_files.end() && next_key->first == it->first)
		{
			++next_key;
			++count;
		}

		// oh no, duplication!
		if(count > 1)
		{
			printf("\n%s exists in multiple mixes:\n", it->second.name);
	
			int prio_count = sizeof(mix_priority) / sizeof(mix_priority[0]);
			int min_prio = prio_count;
			std::unordered_multimap<int32_t, FileEntry>::iterator min_it;

			for(auto it2 = it; it2 != next_key; ++it2)
			{
				// calc priority

				// use uppercased basename for compare
				std::string mix_name = it2->second.mix->Filename;
				auto last_slash = mix_name.find_last_of("/\\");
				if(last_slash != std::string::npos)
					mix_name = mix_name.substr(last_slash + 1);

				std::transform(mix_name.begin(), mix_name.end(), mix_name.begin(), ::toupper);

				int prio = 0;
				for(auto &prio_mix_name : mix_priority)
				{
					if(strcmp(prio_mix_name, mix_name.c_str()) == 0)
						break;
					prio++;
				}

				printf("\t%-12s (prio %2i size %u)\n", it2->second.mix->Filename, prio, it2->second.block->Size);
			
				if(prio < min_prio)
				{
					min_prio = prio;
					min_it = it2;
				}
			}

			// now remove all the lower proprity ones
			for(auto it2 = it; it2 != next_key;)
			{
				if(it2 == min_it)
					++it2;
				else
					it2 = all_files.erase(it2);
			}
		}
	
		it = next_key;
	}

	// go through final list
	std::unordered_map<std::string, uint32_t> size_by_ext;

	for(auto it = all_files.begin(); it != all_files.end(); ++it)
	{
		// add up total sizes
		auto filename = it->second.name;
		const char * ext = nullptr;
		
		if(filename)
			ext = strrchr(filename, '.');

		if(!ext)
			ext = ".UNK";

		// remap all the audio extensions
		if((ext[1] == 'V' || ext[1] == 'R') && ext[2] == '0')
			ext = ".AUD";

		if(strcmp(ext, ".MIX") != 0)
			size_by_ext[ext] += it->second.block->Size;
	}

	// display size breakdown
	std::list<std::pair<std::string, int>> size_list;
	for(auto fs : size_by_ext)
		size_list.emplace_back(fs);

	size_list.sort([](std::pair<std::string, int> &a, std::pair<std::string, int> &b){return a.second > b.second;});

	printf("\nTotal size by type:\n");
	uint32_t total = 0;
	for(auto &fs : size_list)
	{
		printf("\t%s: %9u\n", fs.first.c_str(), fs.second);
		total += fs.second;
	}
	printf("\t ALL: %9u\n", total);

	// now try to re-assemble the files
	std::unordered_map<std::string, MixWriter> output_mixes;
	for(auto it = all_files.begin(); it != all_files.end(); ++it)
	{
		// look up the real file
		uint32_t offset = it->second.block->Offset + it->second.mix->Get_Data_Start();
		auto mix_filename = it->second.mix->Filename;
		auto orig_mix_filename = mix_filename;
		auto parent_it = mix_parents.find(it->second.mix);

		auto filename = it->second.name;
		const char *ext = NULL;
		if(filename)
			ext = strrchr(filename, '.');

		// skip nested mixes, we'll rebuild the container mixes later
		if(ext && strcmp(ext, ".MIX") == 0)
			continue;

		// special case: merge the two MOVIES mixes
		if(strcmp(mix_filename, "MOVIES2.MIX") == 0)
			orig_mix_filename = "MOVIES1.MIX";

		// nested mix, go back to true file
		if(parent_it != mix_parents.end())
		{
			auto parent_mix = std::get<0>(parent_it->second);
			mix_filename = parent_mix->Filename;
		}

		// read ALL the data
		auto data = new uint8_t[it->second.block->Size];
		std::ifstream file(mix_filename, std::ios::binary);

		file.seekg(offset);
		file.read((char *)data, it->second.block->Size);
		output_mixes[orig_mix_filename].addEntry(it->second.block->CRC, it->second.block->Size, data);
	}

	if(!create_directory("./remixed"))
	{
		printf("couldn't create output directory!\n");
		return 1;
	}

	for(auto &file : output_mixes)
	{
		// remove path
		auto name = file.first;
		auto last_slash = name.find_last_of("/\\");
		if(last_slash != std::string::npos)
			name = name.substr(last_slash + 1);

		file.second.write(std::string("./remixed/").append(name).c_str());
	}

	// build the "container" mixes
	std::list<const char *> redalert_files{"LOCAL.MIX", "SPEECH.MIX"};
	std::list<const char *> main_files{"MOVIES1.MIX", "CONQUER.MIX", "RUSSIAN.MIX", "ALLIES.MIX", "SOUNDS.MIX", "SCORES.MIX", "SNOW.MIX", "INTERIOR.MIX", "TEMPERAT.MIX", "GENERAL.MIX"};

	if(hires)
	{
		redalert_files.push_back("HIRES.MIX");
		redalert_files.push_back("NCHIRES.MIX");

		if(editor)
			main_files.push_back("EDHI.MIX");
	}
	else
	{
		redalert_files.push_back("LORES.MIX");
		if(editor)
			main_files.push_back("EDLO.MIX");
	}

	if(editor)
		redalert_files.push_back("EDITOR.MIX");

	MixWriter redalert_mix, main_mix;

	for(auto &filename : main_files)
	{
		std::ifstream file(std::string("./remixed/").append(filename).c_str(), std::ios::binary);
		file.seekg(0, std::ios::end);
		auto size = file.tellg();
		file.seekg(0, std::ios::beg);

		auto data = new uint8_t[size];
		file.read((char *)data, size);
		main_mix.addEntry(CRCEngine()(filename, strlen(filename)), size, data);
	}

	for(auto &filename : redalert_files)
	{
		std::ifstream file(std::string("./remixed/").append(filename).c_str(), std::ios::binary);
		file.seekg(0, std::ios::end);
		auto size = file.tellg();
		file.seekg(0, std::ios::beg);

		auto data = new uint8_t[size];
		file.read((char *)data, size);
		redalert_mix.addEntry(CRCEngine()(filename, strlen(filename)), size, data);
	}

	main_mix.write("./remixed/MAIN.MIX");
	redalert_mix.write("./remixed/REDALERT.MIX");

	// because we unlinked, these are going to leak
	for(auto &file : mix_files)
		delete file;

    return 0;
}

// stubs
// don't care about CDs
int RequiredCD = -1;

int Get_CD_Index(int, int)
{
    return -1;
}

bool Force_CD_Available(int)
{
    return true;
}

// called if Force_CD_Available fails
void Emergency_Exit(int)
{
}

RandomStraw CryptRandom;

extern "C" long Calculate_CRC(void * buffer, long len)
{
	return CRCEngine()(buffer, len);
}

// used by compat wrapper in ccfile
void *Load_Alloc_Data(FileClass &)
{
    return nullptr;
}