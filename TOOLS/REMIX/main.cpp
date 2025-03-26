#include <list>
#include <set>
#include <string>
#include <unordered_map>

#include "base64.h"
#include "ccfile.h"
#include "crc.h"
#include "mixfile.h"
#include "rndstraw.h"

#include "names.h"

using CCMixFile = MixFileClass<CCFileClass>;

static std::unordered_map<int32_t, const char *> name_map;

static PKey *key;

// list state
static MixFileClass<CCFileClass> *last_mix = nullptr;

// keep track of all mix files so we can clean them up
// (because we unlinked the list)
std::set<CCMixFile *> mix_files;

// keep track of nesting
static std::unordered_map<CCMixFile *, CCMixFile *> mix_parents;

struct FileEntry
{
	const char *name;
	CCMixFile *mix;
	CCMixFile::SubBlock *block;
};

static std::unordered_multimap<int32_t, FileEntry> all_files;

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
			printf("\n%s (%s):\n", mix->Filename, parent_it->second->Filename);
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
				mix_parents.emplace(new_mix, mix);
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

		if(remove)
		{
			printf("removing %s in %s\n", it->second.name, mix_filename);
			it = all_files.erase(it);
		}
		else
			++it;
	}

	// go through final list
	std::unordered_map<std::string, uint32_t> size_by_ext;

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
			for(auto it2 = it; it2 != next_key; ++it2)
				printf("\t%s (size %u)\n", it2->second.mix->Filename, it2->second.block->Size);
		}

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
	
		it = next_key;
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