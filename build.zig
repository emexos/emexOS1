const std = @import("std");

pub fn findAsmFiles(gpa: std.mem.Allocator) ![][]const u8 {
    var ret = try std.ArrayList([]const u8).initCapacity(gpa, 10);
    var root = try std.fs.cwd().openDir("src", .{ .iterate = true });
    defer root.close();

    var walker = try root.walk(gpa);
    defer walker.deinit();

    while (try walker.next()) |entry| {
        if (!std.mem.endsWith(u8, entry.path, ".asm")) continue;
        try ret.append(gpa, try std.fmt.allocPrint(gpa, "src/{s}", .{entry.path}));
    }

    return try ret.toOwnedSlice(gpa);
}

pub fn findCFiles(gpa: std.mem.Allocator) ![][]const u8 {
    var ret = try std.ArrayList([]const u8).initCapacity(gpa, 10);
    var root = try std.fs.cwd().openDir("src", .{ .iterate = true });
    defer root.close();

    var walker = try root.walk(gpa);
    defer walker.deinit();

    while (try walker.next()) |entry| {
        if (!std.mem.endsWith(u8, entry.path, ".c")) continue;
        if (std.mem.endsWith(u8, entry.path, "kernel.c")) continue;
        try ret.append(gpa, try std.fmt.allocPrint(gpa, "src/{s}", .{entry.path}));
    }

    return try ret.toOwnedSlice(gpa);
}

pub fn findAllDirs(gpa: std.mem.Allocator) ![][]const u8 {
    var ret = try std.ArrayList([]const u8).initCapacity(gpa, 10);
    try ret.append(gpa, ".");
    var root = try std.fs.cwd().openDir(".", .{ .iterate = true });
    defer root.close();

    var walker = try root.walk(gpa);
    defer walker.deinit();

    while (try walker.next()) |entry| {
        if (entry.kind != .directory) continue;
        if (entry.path[0] == '.') continue;
        try ret.append(gpa, try std.fmt.allocPrint(gpa, "{s}", .{entry.path}));
    }

    return try ret.toOwnedSlice(gpa);
}

pub fn build(b: *std.Build) !void {
    const clean = b.option(bool, "clean", "removes .zig-cahe and zig-out");
    const fetchLimine = b.option(bool, "nofetch", "do not fetch limine");

    const start = b.step("start", "start");

    if (clean != null) {
        const rmCache = b.addRemoveDirTree(b.path(".zig-cache/"));
        start.dependOn(&rmCache.step);
        const rmOut = b.addRemoveDirTree(b.path("zig-out/"));
        start.dependOn(&rmOut.step);
    }

    if (fetchLimine == null) {
        const liminePath = "limine";
        const liminePathRemove = b.addRemoveDirTree(b.path(liminePath));

        const limineGitClone = b.addSystemCommand(&.{
            "git",
            "clone",
            "https://codeberg.org/Limine/Limine.git",
            "--branch=v10.x-binary",
            "--depth=1",
            liminePath,
        });

        limineGitClone.step.dependOn(&liminePathRemove.step);
        start.dependOn(&limineGitClone.step);
    }

    const mkdirOut = b.addSystemCommand(&.{ "mkdir", "-p", "zig-out/bin" });
    mkdirOut.step.dependOn(start);
    b.getInstallStep().dependOn(&mkdirOut.step);

    const target = b.resolveTargetQuery(.{
        .cpu_arch = .x86_64,
        .os_tag = .freestanding,
        .abi = .none,
    });

    const optimize = b.standardOptimizeOption(.{});

    const kernelObj = b.addObject(.{
        .name = "kernel",
        .root_module = b.createModule(.{
            .target = target,
            .optimize = optimize,
            .link_libc = false,
            .sanitize_thread = false,
            .sanitize_c = .off,
        }),
    });

    kernelObj.root_module.sanitize_thread = false;
    kernelObj.root_module.strip = false;
    kernelObj.root_module.red_zone = false;
    kernelObj.root_module.stack_check = false;
    kernelObj.root_module.stack_protector = false;
    kernelObj.root_module.single_threaded = true;
    kernelObj.root_module.pic = false;
    kernelObj.root_module.code_model = .kernel;

    const flags = [_][]const u8{
        "-std=c99",
        "-ffreestanding",
        "-fno-stack-protector",
        "-nostdlib",
        "-fno-builtin",
        "-mno-red-zone",
    };

    kernelObj.addCSourceFile(.{
        .file = b.path("src/kernel/kernel.c"),
        .flags = &flags,
    });

    const includePaths = try findAllDirs(b.allocator);
    defer b.allocator.free(includePaths);

    for (includePaths) |includePath| {
        kernelObj.addIncludePath(b.path(includePath));
    }

    const cfilePaths = try findCFiles(b.allocator);
    defer b.allocator.free(cfilePaths);

    for (cfilePaths) |cfilePath| {
        kernelObj.addCSourceFile(.{
            .file = b.path(cfilePath),
        });
    }

    const asmPaths = try findAsmFiles(b.allocator);

    var asmCompileSteps = try b.allocator.alloc(*std.Build.Step, asmPaths.len);
    defer b.allocator.free(asmCompileSteps);

    for (asmPaths, 0..) |asmPath, idx| {
        const asmOutPath = try std.fmt.allocPrint(b.allocator, "zig-out/bin/{s}.o", .{std.fs.path.stem(asmPath)});
        defer b.allocator.free(asmOutPath);
        const asmOutCmd = b.addSystemCommand(&.{ "nasm", "-f", "elf64", asmPath, "-o", asmOutPath });
        asmOutCmd.step.dependOn(&mkdirOut.step);
        asmCompileSteps[idx] = &asmOutCmd.step;
    }

    const asmLoadObj = b.step("asm load o", "asm o files should be created");
    for (asmCompileSteps) |asmCompileStep| {
        asmLoadObj.dependOn(asmCompileStep);
    }

    var asmLoadSteps = try b.allocator.alloc(*std.Build.Step, asmPaths.len);
    defer b.allocator.free(asmLoadSteps);

    for (asmPaths, 0..) |asmPath, idx| {
        const stepName = try std.fmt.allocPrint(
            b.allocator,
            "asmLoad{d}",
            .{idx},
        );
        const stepAsmLoad = b.step(stepName, "should be loaded asm o");
        stepAsmLoad.dependOn(asmLoadObj);
        asmLoadSteps[idx] = stepAsmLoad;

        const asmOutPath = try std.fmt.allocPrint(b.allocator, "zig-out/bin/{s}.o", .{std.fs.path.stem(asmPath)});
        defer b.allocator.free(asmOutPath);
        kernelObj.addObjectFile(b.path(asmOutPath));
    }

    const asmLoadComplete = b.step("asmLoadComplete", "should be completed load o");
    for (asmLoadSteps) |asmLoadStep| {
        asmLoadComplete.dependOn(asmLoadStep);
    }

    kernelObj.step.dependOn(asmLoadComplete);
    b.getInstallStep().dependOn(&kernelObj.step);

    const exe = b.addExecutable(.{
        .name = "kernel.elf",
        .root_module = b.createModule(.{
            .target = target,
            .optimize = optimize,
            .link_libc = false,
            .sanitize_thread = false,
            .sanitize_c = .off,
        }),
    });

    exe.addObject(kernelObj);

    exe.entry = .disabled;
    exe.bundle_compiler_rt = false;
    exe.root_module.sanitize_thread = false;
    exe.link_gc_sections = true;
    exe.link_function_sections = true;
    exe.pie = false;
    exe.root_module.import_table.clearAndFree(b.allocator);
    exe.setLinkerScript(b.path("src/kernel/linker.ld"));

    b.installArtifact(exe);
    b.getInstallStep().dependOn(&exe.step);

    const isoBuildStep = b.step("ISO BUILD", "create iso file");

    const isoDir = "zig-out/iso";
    const isoName = "zig-out/emexos.iso";
    const kernelPath = "zig-out/bin/kernel.elf";

    const isoClean = b.addRemoveDirTree(b.path(isoDir));

    const isoCreatePath = b.addSystemCommand(&.{
        "mkdir",
        "-p",
        isoDir ++ "/boot/limine",
        isoDir ++ "/EFI/BOOT",
    });
    isoCreatePath.step.dependOn(&isoClean.step);

    const copyFiles = b.step("COPY FILES", "coping files");

    const isoCopy = b.addSystemCommand(&.{ "cp", kernelPath, isoDir ++ "/boot/" });
    isoCopy.step.dependOn(&isoCreatePath.step);
    isoCopy.step.dependOn(&exe.step);

    const cnfCopy = b.addSystemCommand(&.{ "cp", "limine.conf", isoDir ++ "/boot/limine/" });
    cnfCopy.step.dependOn(&isoCreatePath.step);

    const limineCopy = b.addSystemCommand(&.{
        "cp",
        "limine/limine-bios.sys",
        "limine/limine-bios-cd.bin",
        "limine/limine-uefi-cd.bin",
        isoDir ++ "/boot/limine/",
    });
    limineCopy.step.dependOn(&isoCreatePath.step);
    limineCopy.step.dependOn(start);

    const efiCopy = b.addSystemCommand(&.{
        "sh",
        "-c",
        try std.fmt.allocPrint(b.allocator, "\\cp limine/BOOTIA32.EFI {s}/EFI/BOOT\ncp limine/BOOTX64.EFI {s}/EFI/BOOT/", .{ isoDir, isoDir }),
    });
    efiCopy.step.dependOn(start);
    efiCopy.step.dependOn(&isoCreatePath.step);

    copyFiles.dependOn(&isoCopy.step);
    copyFiles.dependOn(&cnfCopy.step);
    copyFiles.dependOn(&limineCopy.step);
    copyFiles.dependOn(&efiCopy.step);

    const isoBuild = b.addSystemCommand(&.{
        "xorriso",
        "-as",
        "mkisofs",
        "-b",
        "boot/limine/limine-bios-cd.bin",
        "-no-emul-boot",
        "-boot-load-size",
        "4",
        "-boot-info-table",
        "--efi-boot",
        "boot/limine/limine-uefi-cd.bin",
        "-efi-boot-part",
        "--efi-boot-image",
        "--protective-msdos-label",
        isoDir,
        "-o",
        isoName,
    });
    isoBuild.step.dependOn(copyFiles);

    isoBuildStep.dependOn(&isoBuild.step);

    const arch = "x86_64"; // или определите динамически
    const qemuRun = b.addSystemCommand(&.{
        "qemu-system-" ++ arch,
        "-m",
        "512",
        "-cdrom",
        isoName,
        "-serial",
        "stdio",
    });
    qemuRun.step.dependOn(isoBuildStep);

    b.getInstallStep().dependOn(isoBuildStep);
    b.getInstallStep().dependOn(&qemuRun.step);
}
