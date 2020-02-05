#include "PrecompiledHeader.h"
#include "Script.h"
#include "Memory.h"

#include "sol/sol.hpp"

sol::state lua;

sol::function ExecuteOnceOnLoad;
sol::function ExecuteContinuously;

namespace
{
	u8 ReadByte(u32 addr)
	{
		return memRead8(addr);
	}

	u16 Read2Byte(u32 addr)
    {
        return memRead16(addr);
    }

	u32 Read4Byte(u32 addr)
    {
        return memRead32(addr);
    }

	void WriteByte(u32 addr, u8 val)
	{
        if (memRead8(addr) != (u8)val)
			memWrite8(addr, (u8)val);
	}

	void Write2Byte(u32 addr, u16 val)
    {
        if (memRead16(addr) != (u16)val)
			memWrite16(addr, (u16)val);
    }

	void Write4Byte(u32 addr, u32 val)
    {
        if (memRead32(addr) != (u32)val)
			memWrite32(addr, (u32)val);
    }

	std::vector<u8> ReadBuffer(u32 addr, u32 size)
	{
        std::vector<u8> buffer(size);
        for (size_t i = 0; i < size; i++) {
            buffer.at(i) = memRead8(addr);
            addr++;
		}

		return buffer;
	}
}

void ExportFunctionCalls()
{
    // Export memory functions to lua script
    lua.set_function("ReadByte", ReadByte);
    lua.set_function("Read2Byte", Read2Byte);
    lua.set_function("Read4Byte", Read4Byte);

    lua.set_function("WriteByte", WriteByte);
    lua.set_function("Write2Byte", Write2Byte);
    lua.set_function("Write4Byte", Write4Byte);

    lua.set_function("ReadBuffer", ReadBuffer);
}

bool InitScript(wxString path)
{
    lua.open_libraries(sol::lib::base, sol::lib::package);
	
	ExportFunctionCalls();

    // Parse the main script
    lua.do_file(path.ToStdString());

    // Assign function calls to lua functions
    ExecuteOnceOnLoad = lua["ExecuteOnceOnLoad"];
    ExecuteContinuously = lua["ExecuteContinuously"];

    if (!ExecuteOnceOnLoad || !ExecuteContinuously)
        return false;

    return true;
}

bool LoadScriptFromDir(wxString name, const wxDirName &folderName, const wxString &friendlyName)
{
    if (!folderName.Exists()) {
        Console.WriteLn(Color_Red, L"The %s folder ('%s') is inaccessible. Skipping...", WX_STR(friendlyName), WX_STR(folderName.ToString()));
        return 0;
    }

    wxString pathName = Path::Combine(folderName, name.MakeUpper() + L".lua");
    return InitScript(pathName);
}

void ExecuteScript(script_place_type place)
{
    if (!ExecuteOnceOnLoad || !ExecuteContinuously)
        return;

    switch (place) {
        case SPT_ONCE_ON_LOAD:
            ExecuteOnceOnLoad();
            break;
        case SPT_CONTINOUSLY:
            ExecuteContinuously();
            break;
    }
}