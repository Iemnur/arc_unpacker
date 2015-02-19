// ANM file
//
// Company:   Team Shanghai Anime
// Engine:    -
// Extension: .anm
//
// Known games:
// - Touhou 06 - The Embodiment of Scarlet Devil

#include "formats/gfx/anm_archive.h"
#include "formats/image.h"
#include "string/itos.h"

namespace
{
    const std::string texture_magic("THTX", 4);

    typedef struct
    {
        size_t width;
        size_t height;
        size_t format;
        std::string name1;
        std::string name2;
        int version;
        size_t texture_offset;
        bool has_data;
        std::vector<size_t> sprite_offsets;
        std::vector<size_t> script_offsets;
    } TableEntry;

    typedef std::vector<std::unique_ptr<TableEntry>> Table;

    std::string read_name(IO &file_io, size_t offset)
    {
        size_t old_offset = file_io.tell();
        file_io.seek(offset);
        std::string name = file_io.read_until_zero();
        file_io.seek(old_offset);
        return name;
    }

    Table read_table(IO &file_io)
    {
        Table table;
        uint32_t start_offset = 0;
        while (true)
        {
            std::unique_ptr<TableEntry> table_entry(new TableEntry);
            file_io.seek(start_offset);
            size_t sprite_count = file_io.read_u32_le();
            size_t script_count = file_io.read_u32_le();
            file_io.skip(4);

            table_entry->width = file_io.read_u32_le();
            table_entry->height = file_io.read_u32_le();
            table_entry->format = file_io.read_u32_le();
            file_io.skip(4);

            size_t name_offset1 = start_offset + file_io.read_u32_le();
            file_io.skip(4);
            size_t name_offset2 = start_offset + file_io.read_u32_le();
            table_entry->name1 = read_name(file_io, name_offset1);
            table_entry->name2 = read_name(file_io, name_offset2);

            table_entry->version = file_io.read_u32_le();
            file_io.skip(4);
            table_entry->texture_offset = start_offset + file_io.read_u32_le();
            table_entry->has_data = file_io.read_u32_le() > 0;

            size_t next_offset = start_offset + file_io.read_u32_le();
            file_io.skip(4);

            for (size_t i = 0; i  < sprite_count; i ++)
            {
                table_entry->sprite_offsets.push_back(
                    start_offset + file_io.read_u32_le());
            }

            for (size_t i = 0; i < script_count; i ++)
            {
                table_entry->script_offsets.push_back(
                    start_offset + file_io.read_u32_le());
            }

            table.push_back(std::move(table_entry));
            if (next_offset == start_offset)
                break;
            start_offset = next_offset;
        }
        return table;
    }

    std::unique_ptr<File> read_texture(IO &file_io, TableEntry &table_entry)
    {
        if (!table_entry.has_data)
            return nullptr;

        file_io.seek(table_entry.texture_offset);
        if (file_io.read(texture_magic.size()) != texture_magic)
            throw std::runtime_error("Corrupt texture data");
        file_io.skip(2);
        int format = file_io.read_u16_le();
        size_t width = file_io.read_u16_le();
        size_t height = file_io.read_u16_le();
        size_t data_size = file_io.read_u32_le();

        size_t pixel_data_size = width * height * 4;
        std::unique_ptr<char[]> pixel_data(new char[pixel_data_size]);
        uint32_t *pixel_ptr = reinterpret_cast<uint32_t*>(pixel_data.get());
        switch (format)
        {
            case 1:
                //8 8 8 8 BGRA
                for (size_t y = 0; y < height; y ++)
                {
                    for (size_t x = 0; x < width; x ++)
                        *pixel_ptr ++ = file_io.read_u32_le();
                }
                break;

            case 3:
                //5 6 5 RGB
                for (size_t y = 0; y < height; y ++)
                {
                    for (size_t x = 0; x < width; x ++)
                    {
                        uint16_t word = file_io.read_u16_le();
                        *pixel_ptr ++
                            = ((word & 0x1f) << 3)
                            | ((word & 0x7e0) << 5)
                            | ((word & 0xf800) << 8)
                            | 0xff000000;
                    }
                }
                break;

            case 5:
                //4 4 4 4 BGRA
                for (size_t y = 0; y < height; y ++)
                {
                    for (size_t x = 0; x < width; x ++)
                    {
                        uint16_t word = file_io.read_u16_le();
                        *pixel_ptr ++
                            = ((word & 0xf) << 4)
                            | ((word & 0xf0) << 8)
                            | ((word & 0xf00) << 12)
                            | ((word & 0xf000) << 16);
                    }
                }
                break;

            default:
                throw std::runtime_error("Unknown format: " + itos(format));
        }

        std::unique_ptr<File> subfile(new File);
        subfile->name = table_entry.name1;
        std::unique_ptr<Image> image = Image::from_pixels(
            width,
            height,
            std::string(pixel_data.get(), pixel_data_size),
            PixelFormat::IMAGE_PIXEL_FORMAT_BGRA);
        image->update_file(*subfile);
        return subfile;
    }
}

#include <iostream>
void AnmArchive::unpack_internal(File &file, FileSaver &file_saver) const
{
    Table table = read_table(file.io);
    for (auto &table_entry : table)
    {
        // Ignore both the scripts and sprites and extract raw texture data.
        auto subfile = read_texture(file.io, *table_entry);
        if (subfile)
            file_saver.save(std::move(subfile));
    }
}
