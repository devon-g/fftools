const std = @import("std");

/// Decompress fastfile data using zlib
fn ff_decompress(allocator: std.mem.Allocator, data: []const u8) ![]const u8 {
    // Ignore first 12 bytes (ff magic header)
    var data_stream = std.io.fixedBufferStream(data[12..]);

    var data_decompressed = std.ArrayList(u8).init(allocator);
    defer data_decompressed.deinit();
    try std.compress.zlib.decompress(data_stream.reader(), data_decompressed.writer());

    return allocator.alloc(u8, data_decompressed.items.len);
}

/// Load compressed bytes from file path and confirm ff file format
fn ff_load(allocator: std.mem.Allocator, path: []const u8) ![]const u8 {
    std.debug.print("Loading file: {s}\n", .{ path });

    const file = try std.fs.cwd().openFile(path, .{});
    defer file.close();

    const stat = try file.stat();
    const data = try file.readToEndAlloc(allocator, stat.size);

    // FF magic header: IWffu100\x83\x01\x00\x00
    const ff_magic: []const u8 = &[_]u8{ 0x49, 0x57, 0x66, 0x66, 0x75, 0x31, 0x30, 0x30, 0x83, 0x01, 0x00, 0x00 };
    if (std.mem.eql(u8, ff_magic, data[0..12])) {
        std.debug.print("CoD: WaW fastfile format detected\n", .{});
    } else {
        return error.IncorrectFileFormat;
    }

    return data;
}

pub fn main() !void {
    var gpa = std.heap.GeneralPurposeAllocator(.{}){};
    defer _ = gpa.deinit();
    const allocator = gpa.allocator();
    
    var args = try std.process.argsWithAllocator(allocator);
    defer args.deinit();

    // One arg[1] is path to ff file
    _ = args.skip();
    var path: []const u8 = ""; 
    if (args.next()) |arg| {
        path = arg;
    } else {
        std.debug.print("Usage: zff <ff_path>\n", .{});
        return;
    }

    const ff = try ff_load(allocator, path);
    defer allocator.free(ff);

    const ff_body = try ff_decompress(allocator, ff);
    defer allocator.free(ff_body);

    // TODO: Parse body of ff
    //  * collection of files and directories?
    //  * any way to tell how long chunks of file data are?
    //  * generate file tree to start?
}
