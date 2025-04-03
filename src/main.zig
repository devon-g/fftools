const std = @import("std");

pub fn main() !void {
    var gpa = std.heap.GeneralPurposeAllocator(.{}){};
    defer _ = gpa.deinit();
    const allocator = gpa.allocator();
    
    var args = try std.process.argsWithAllocator(allocator);
    defer args.deinit();

    // Skip program path arg
    _ = args.skip();

    // One arg expected 
    var path: []const u8 = ""; 
    if (args.next()) |arg| {
        path = arg;
    } else {
        std.debug.print("Usage: zff <ff_path>\n", .{});
        return;
    }

    const file = try std.fs.cwd().openFile(path, .{});
    defer file.close();

    const stat = try file.stat();
    const data = try file.readToEndAlloc(allocator, stat.size);
    defer allocator.free(data);

    const data_header = data[0..12];
    const data_compressed = data[12..];
    var data_stream = std.io.fixedBufferStream(data_compressed);

    var data_decompressed = std.ArrayList(u8).init(allocator);
    defer data_decompressed.deinit();

    // FF magic header: IWffu100\x83\x01\x00\x00
    const ff_magic: []const u8 = &[_]u8{ 0x49, 0x57, 0x66, 0x66, 0x75, 0x31, 0x30, 0x30, 0x83, 0x01, 0x00, 0x00 };

    // Decompress rest of file if it is indeed a fastfile format
    // Rest of file is zlib compressed data
    if (std.mem.eql(u8, ff_magic, data_header)) {
        try std.compress.zlib.decompress(data_stream.reader(), data_decompressed.writer());
    } else {
        std.debug.print("CoD: WaW FastFile format not detected\n", .{});
    }
}
