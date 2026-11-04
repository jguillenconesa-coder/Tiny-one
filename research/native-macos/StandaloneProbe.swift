// SPDX-License-Identifier: AGPL-3.0-only
import Foundation
import CoreGraphics

// Observe a real standalone process/window. No acoustic or device claim.
guard CommandLine.arguments.count == 2 else { exit(2) }
let app = Process()
app.executableURL = URL(fileURLWithPath: CommandLine.arguments[1])
do { try app.run() } catch { fputs("Standalone launch failed: \(error)\n", stderr); exit(1) }
defer { if app.isRunning { app.terminate() } }
Thread.sleep(forTimeInterval: 8)
guard app.isRunning else { fputs("Standalone exited during startup\n", stderr); exit(1) }
let windows = CGWindowListCopyWindowInfo(.optionAll, kCGNullWindowID) as? [[String: Any]] ?? []
let owned = windows.filter {
    ($0[kCGWindowOwnerPID as String] as? NSNumber)?.int32Value == app.processIdentifier &&
    ($0[kCGWindowLayer as String] as? NSNumber)?.intValue == 0
}
guard !owned.isEmpty else { fputs("Standalone has no observable application window\n", stderr); app.terminate(); exit(1) }
print("PASS: native standalone stayed running and created \(owned.count) application window(s)")
