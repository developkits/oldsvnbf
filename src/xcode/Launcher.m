#import "Launcher.h"
#import "ConsoleView.h"
#include <stdlib.h>
#include <unistd.h> /* _exit() */
#include <util.h> /* forkpty() */

// User default keys
#define dkVERSION @"version"
#define dkFULLSCREEN @"fullscreen"
#define dkFSAA @"fsaa"
#define dkSHADER @"shader"
#define dkRESOLUTION @"resolution"
#define dkADVANCEDOPTS @"advancedOptions"
#define dkSERVEROPTS @"server_options"
#define dkDESCRIPTION @"server_description"
#define dkPASSWORD @"server_password"
#define dkMAXCLIENTS @"server_maxclients"

//If you make a MOD then please change this, the bundle indentifier, the file extensions (.ogz, .dmo), and the url registration.
#define kBLOODFRONTIER @"bloodfrontier"

//tab names, i.e. image names (text is localised)
#define tkMAIN @"Main"
#define tkMAPS @"Maps"
#define tkKEYS @"Keys"
#define tkSERVER @"Server"

// property keys
#define pkServerRunning @"serverRunning"

@interface NSString(Extras)
@end
@implementation NSString(Extras)
- (NSString*)expand 
{
    NSMutableString *str = [NSMutableString string];
    [str setString:self];
    [str replaceOccurrencesOfString:@":s" withString:kBLOODFRONTIER options:0 range:NSMakeRange(0, [str length])]; 
    return str;
}
@end


@interface NSUserDefaults(Extras) // prevent strings that are "(null)"
- (NSString*)nonNullStringForKey:(NSString*)key;
@end
@implementation NSUserDefaults(Extras)
- (NSString*)nonNullStringForKey:(NSString*)key 
{
    NSString *result = [self stringForKey:key];
    return result ? result : @"";
}
@end


@interface Map : NSObject 
{
    NSString *path;
    BOOL demo, user;
}
@end
@implementation Map
- (id)initWithPath:(NSString*)aPath user:(BOOL)aUser demo:(BOOL)aDemo
{
    if ((self = [super init])) 
    {
        path = [[aPath stringByDeletingPathExtension] retain];
        user = aUser;
        demo = aDemo;
    }
    return self;
}
- (void)dealloc 
{
    [path release];
    [super dealloc];
}
- (NSString*)path { return demo ? [NSString stringWithFormat:@"-xdemo \"%@\"", path] : path; } // minor hack
- (NSString*)name { return [path lastPathComponent]; }
- (NSImage*)image 
{ 
    NSImage *image = [[[NSImage alloc] initWithContentsOfFile:[path stringByAppendingString:@".png"]] autorelease]; 
    if (image) return image;
    return demo ? [NSImage imageNamed:tkMAIN] : [NSImage imageNamed:tkMAPS];
}
- (NSString*)text 
{
    NSData *data = [NSData dataWithContentsOfFile:[path stringByAppendingString:@".txt"]];
    if (data) 
    {
        NSString *text = [[[NSString alloc] initWithData:data encoding:NSUTF8StringEncoding] autorelease];
        if (text) return text;
    }
    return demo ? @"Recorded demo data" : @"";
}
- (void)setText:(NSString*)text { } // strangely textfield believes it's editable

+ (NSString*)tick:(BOOL)ticked 
{ 
    unichar tickCh = 0x2713; 
    return ticked ? [NSString stringWithCharacters:&tickCh length:1] : @"";
}
- (NSString*)tickIfExists:(NSString*)ext { return [Map tick:[[NSFileManager defaultManager] fileExistsAtPath:[path stringByAppendingString:ext]]]; }
- (NSString*)hasImage { return [self tickIfExists:@".png"]; }
- (NSString*)hasText { return [self tickIfExists:@".txt"]; }
- (NSString*)hasCfg { return [self tickIfExists:@".cfg"]; }
- (NSString*)user { return [Map tick:user]; }
@end


static int numberForKey(CFDictionaryRef desc, CFStringRef key) 
{
    CFNumberRef value;
    int num = 0;
    if ((value = CFDictionaryGetValue(desc, key)) == NULL) return 0;
    CFNumberGetValue(value, kCFNumberIntType, &num);
    return num;
}


@interface Launcher(ToolBar)
@end
@implementation Launcher(ToolBar)

- (void)switchViews:(NSToolbarItem *)item 
{
    NSView *views[] = {view1, view2, view3, view4, view5};
    NSView *prefsView = views[[item tag]-1];
    
    // to stop flicker, make a temp blank view
    NSView *tempView = [[NSView alloc] initWithFrame:[[window contentView] frame]];
    [window setContentView:tempView];
    [tempView release];
    
    // mojo to get the right frame for the new window
    NSRect newFrame = [window frame];
    newFrame.size.height = [prefsView frame].size.height + ([window frame].size.height - [[window contentView] frame].size.height);
    newFrame.size.width = [prefsView frame].size.width;
    newFrame.origin.y += ([[window contentView] frame].size.height - [prefsView frame].size.height);
    
    // set the frame to newFrame and animate it
    [window setFrame:newFrame display:YES animate:YES];
    // set the main content view to the new view
    [window setContentView:prefsView];
    [window setContentMinSize:[prefsView bounds].size];
}

- (void)initToolBar 
{
    toolBarItems = [[NSMutableDictionary alloc] init];
    NSEnumerator *e = [[self toolbarDefaultItemIdentifiers:nil] objectEnumerator];
    NSString *identifier;
    while(identifier = [e nextObject])
    {
        NSToolbarItem *item = [[NSToolbarItem alloc] initWithItemIdentifier:identifier];
        int tag = [identifier intValue];
        NSString *name = identifier;
        SEL action = @selector(showHelp:);
        id target = NSApp;
        if (tag != 0) {
            NSString *names[] = {tkMAIN, tkMAPS, tkKEYS, tkSERVER};
            name = names[tag-1];
            action = @selector(switchViews:);
            target = self;
        }
        [item setTag:tag];
        [item setTarget:target];
        [item setAction:action];
        [item setLabel:NSLocalizedString(name, @"")];
        [item setImage:[NSImage imageNamed:name]];
        [toolBarItems setObject:item forKey:identifier];
        [item release];
    }
    NSToolbar *toolbar = [[NSToolbar alloc] initWithIdentifier:@""];
    [toolbar setDelegate:self]; 
    [toolbar setAllowsUserCustomization:NO]; 
    [toolbar setAutosavesConfiguration:NO];  
    [window setToolbar:toolbar]; 
    [toolbar release];
    if ([window respondsToSelector:@selector(setShowsToolbarButton:)]) [window setShowsToolbarButton:NO]; //10.4+
    
    //select the first by default
    NSToolbarItem *first = [toolBarItems objectForKey:[[self toolbarDefaultItemIdentifiers:nil] objectAtIndex:0]];
    [toolbar setSelectedItemIdentifier:[first itemIdentifier]];
    [self switchViews:first]; 
}

#pragma mark toolbar delegate methods

- (NSToolbarItem *)toolbar:(NSToolbar *)toolbar itemForItemIdentifier:(NSString *)itemIdentifier willBeInsertedIntoToolbar:(BOOL)flag 
{
    return [toolBarItems objectForKey:itemIdentifier];
}

- (NSArray *)toolbarAllowedItemIdentifiers:(NSToolbar*)theToolbar 
{
    return [self toolbarDefaultItemIdentifiers:theToolbar];
}

- (NSArray *)toolbarDefaultItemIdentifiers:(NSToolbar*)toolbar 
{
    NSMutableArray *array = (NSMutableArray *)[self toolbarSelectableItemIdentifiers:toolbar];
    [array addObject:NSToolbarFlexibleSpaceItemIdentifier];
    [array addObject:@"Help"];
    return array;
}

- (NSArray *)toolbarSelectableItemIdentifiers: (NSToolbar *)toolbar 
{
    NSMutableArray *array = [NSMutableArray array];
    NSView *views[] = {view1, view2, view3, view4, view5};
    int i;
    for(i = 0; i < sizeof(views)/sizeof(NSView*); i++) if (views[i]) [array addObject:[NSString stringWithFormat:@"%d", i+1]];
    return array;
}
@end


@implementation Launcher

- (void)initPaths 
{
    NSString *path = [[[NSBundle mainBundle] bundlePath] stringByAppendingPathComponent:@"Contents/gamedata"];
    NSFileManager *fm = [NSFileManager defaultManager];
    if ([fm fileExistsAtPath:path]) 
    {
        dataPath = [path retain];
        appPath = [[[NSBundle bundleWithPath:[path stringByAppendingPathComponent:[@":s.app" expand]]] executablePath] retain];
    } 
    else  // development setup
    {
        // binary for engine is alongside the launcher
        path = [[[NSBundle mainBundle] bundlePath] stringByDeletingLastPathComponent];
        appPath = [[[NSBundle bundleWithPath:[path stringByAppendingPathComponent:[@":s.app" expand]]] executablePath] retain];
        if (![fm fileExistsAtPath:appPath]) NSLog(@"need to build engine as well as the launcher");
        
        // search up the folder till find a folder containing data, or a game application containing packages
        dataPath = nil;
        while ([path length] > 1) 
        {
            path = [path stringByDeletingLastPathComponent];
            NSString *probe = [[path stringByAppendingPathComponent:[@":s.app" expand]] stringByAppendingPathComponent:@"Contents/gamedata"];
            if ([fm fileExistsAtPath:[probe stringByAppendingPathComponent:@"data"]]) 
            {
                NSLog(@"game download folder structure detected - consider using svn if you really want to develop...");
                dataPath = [probe retain];
                break;
            } 
            else if ([fm fileExistsAtPath:[path stringByAppendingPathComponent:@"data"]]) 
            {
                NSLog(@"svn folder structure detected");
                dataPath = [path retain];
                break;
            }        
        }
    }
    // userpath: directory where user files are kept - typically /Users/<name>/Application Support/bloodfrontier
    FSRef folder;
    path = nil;
    if (FSFindFolder(kUserDomain, kApplicationSupportFolderType, NO, &folder) == noErr) 
    {
        CFURLRef url = CFURLCreateFromFSRef(kCFAllocatorDefault, &folder);
        path = [(NSURL *)url path];
        CFRelease(url);
        path = [path stringByAppendingPathComponent:kBLOODFRONTIER];
        NSFileManager *fm = [NSFileManager defaultManager];
        if (![fm fileExistsAtPath:path]) [fm createDirectoryAtPath:path attributes:nil]; // ensure it exists    
    }
    userPath = [path retain];    
}


- (void)addResolutionsForDisplay:(CGDirectDisplayID)dspy 
{
    CFIndex i, cnt;
    CFArrayRef modeList = CGDisplayAvailableModes(dspy);
    if (modeList == NULL) return;
    cnt = CFArrayGetCount(modeList);
    for (i = 0; i < cnt; i++) 
    {
        CFDictionaryRef mode = CFArrayGetValueAtIndex(modeList, i);
        NSString *title = [NSString stringWithFormat:@"%i x %i", numberForKey(mode, kCGDisplayWidth), numberForKey(mode, kCGDisplayHeight)];
        if (![resolutions itemWithTitle:title]) [resolutions addItemWithTitle:title];
    }   
}

- (void)initResolutions 
{
    const int kMaxDisplays = 16;
    CGDirectDisplayID display[kMaxDisplays];
    CGDisplayCount numDisplays;
    [resolutions removeAllItems];
    if (CGGetActiveDisplayList(kMaxDisplays, display, &numDisplays) == CGDisplayNoErr) 
    {
        CGDisplayCount i;
        for (i = 0; i < numDisplays; i++) [self addResolutionsForDisplay:display[i]];
    }
    [resolutions selectItemAtIndex: [[NSUserDefaults standardUserDefaults] integerForKey:dkRESOLUTION]];    
}

/* build key array from config data */
-(NSArray *)getKeys:(NSDictionary *)dict 
{   
    NSMutableArray *arr = [NSMutableArray array];
    NSEnumerator *e = [dict keyEnumerator];
    NSString *key;
    while ((key = [e nextObject])) 
    {
        int pos = [key rangeOfString:@"bind."].location;
        if (pos == NSNotFound || pos > 5) continue;
        [arr addObject:[NSDictionary dictionaryWithObjectsAndKeys: // keys as used in nib
            [key substringFromIndex:pos+5], @"key",
            [key substringToIndex:pos], @"mode",
            [dict objectForKey:key], @"action",
            nil]];
    }
    return arr;
}

/*
 * extract a dictionary from the config files containing:
 * - name, team, gamma strings
 * - bind/editbind '.' key strings
 */
-(NSDictionary *)readConfigFiles 
{
    NSMutableDictionary *dict = [[NSMutableDictionary alloc] init];
    [dict setObject:@"" forKey:@"name"]; //ensure these entries are never nil
    [dict setObject:@"" forKey:@"team"]; 
    
    NSString *files[] = {@"config.cfg", @"autoexec.cfg"};
    int i;
    for(i = 0; i < sizeof(files)/sizeof(NSString*); i++) 
    {
        NSString *file = [userPath stringByAppendingPathComponent:files[i]];
        
        NSArray *lines = [[NSString stringWithContentsOfFile:file] componentsSeparatedByString:@"\n"];
        
        if (i==0 && !lines)  // special case when first run (config.cfg won't exist yet)
        { 
            file = [dataPath stringByAppendingPathComponent:@"data/defaults.cfg"];
            lines = [[NSString stringWithContentsOfFile:file] componentsSeparatedByString:@"\n"];
        }
        
        NSString *line; 
        NSEnumerator *e = [lines objectEnumerator];
        while (line = [e nextObject]) 
        {
            NSScanner *scanner = [NSScanner scannerWithString:line];
            [scanner setCharactersToBeSkipped:[NSCharacterSet characterSetWithRange:NSMakeRange(0, 0)]];
            
            NSString *type = NULL;
            [scanner scanCharactersFromSet:[NSCharacterSet whitespaceCharacterSet] intoString:NULL];
            if ([scanner scanLocation] != 0) continue; // shouldn't be indented
            [scanner scanUpToCharactersFromSet:[NSCharacterSet whitespaceCharacterSet] intoString:&type];
            if (!type) continue;
            
            NSString *value = NULL;
            [scanner scanCharactersFromSet:[NSCharacterSet whitespaceCharacterSet] intoString:NULL];
            if ([scanner scanString:@"\"" intoString:NULL]) 
            {
                [scanner scanUpToString:@"\"" intoString:&value];
                [scanner scanString:@"\"" intoString:NULL];
            } 
            else 
            {
                [scanner scanUpToCharactersFromSet:[NSCharacterSet whitespaceCharacterSet] intoString:&value];
            }
            if (!value) continue;
            
            [scanner scanCharactersFromSet:[NSCharacterSet whitespaceCharacterSet] intoString:NULL];
            int p = [scanner scanLocation];
            NSString *remainder = (p < [line length]) ? [line substringFromIndex:p] : @""; 
           
            
            if ([type isEqualToString:@"name"] || [type isEqualToString:@"team"] || [type isEqualToString:@"gamma"]) 
                [dict setObject:value forKey:type];
            else if ([type isEqualToString:@"bind"] || [type isEqualToString:@"editbind"] || [type isEqualToString:@"specbind"]) 
                [dict setObject:remainder forKey:[NSString stringWithFormat:@"%@.%@", type, value]];
        }
    }
    return [dict autorelease];
}

- (BOOL)serverRunning { return server != -1; }

- (void)killServer 
{
    if (server > 0) kill(server, SIGKILL); //@WARNING - you do NOT want a 0 or -1 to be accidentally sent a  kill!
    [self willChangeValueForKey:pkServerRunning];
    server = -1;
    [self didChangeValueForKey:pkServerRunning];
    [multiplayer setTitle:NSLocalizedString(@"Start", @"")];
    [console appendText:@"\n \n"];
}

- (void)serverDataAvailable:(NSNotification *)note
{
    NSFileHandle *taskOutput = [note object];
    NSData *data = [[note userInfo] objectForKey:NSFileHandleNotificationDataItem];
    
    if (data && [data length])
    {
        NSString *text = [[NSString alloc] initWithData:data encoding:NSASCIIStringEncoding];
        [console appendText:text];
        [text release];                 
        [taskOutput readInBackgroundAndNotify]; //wait for more data
    }
    else
    {
        NSNotificationCenter *nc = [NSNotificationCenter defaultCenter];
        [nc removeObserver:self name:NSFileHandleReadCompletionNotification object:taskOutput];
        close([taskOutput fileDescriptor]);
        [self killServer];
    }
}

- (BOOL)launchGame:(NSArray *)args 
{
    BOOL okay = YES;
    
    if ([args containsObject:@"-ss3"])
    {
        if ([self serverRunning]) return NO; // server is already running
        
        const char **argv = (const char**)malloc(sizeof(char*)*([args count] + 2)); //{path, <args>, NULL};
        argv[0] = [appPath fileSystemRepresentation];        
        argv[[args count]+1] = NULL;
        int i;
        for(i = 0; i < [args count]; i++) argv[i+1] = [[args objectAtIndex:i] UTF8String];  
        
        int fdm;
        NSString *fail = [NSLocalizedString(@"ServerAlertMesg", nil) expand];
        switch ( (server = forkpty(&fdm, NULL, NULL, NULL)) ) // forkpty so can reliably grab SDL console
        { 
            case -1:
                [console appendLine:fail];
                [self killServer];
                okay = NO;
                break;
            case 0: // child
                chdir([userPath fileSystemRepresentation]);
                if (execv(argv[0], (char*const*)argv) == -1) fprintf(stderr, "%s\n", [fail UTF8String]);
                _exit(0);
            default: // parent
                [self willChangeValueForKey:pkServerRunning];
                // changed by forpty
                [self didChangeValueForKey:pkServerRunning];
                
                [multiplayer setTitle:NSLocalizedString(@"Stop", @"")];
                NSFileHandle *taskOutput = [[NSFileHandle alloc] initWithFileDescriptor:fdm];
                NSNotificationCenter *nc = [NSNotificationCenter defaultCenter];
                [nc addObserver:self selector:@selector(serverDataAvailable:) name:NSFileHandleReadCompletionNotification object:taskOutput];
                [taskOutput readInBackgroundAndNotify];
                break;
        }
        free(argv);
    } 
    else 
    {
        NS_DURING
            NSTask *task = [[NSTask alloc] init];
            [task setLaunchPath:appPath];
            [task setCurrentDirectoryPath:dataPath];
            [task setArguments:args];
            [task setEnvironment:[NSDictionary dictionaryWithObjectsAndKeys: 
                @"1", @"SDL_SINGLEDISPLAY",
                @"1", @"SDL_ENABLEAPPEVENTS", nil
            ]]; // makes Command-H, Command-M and Command-Q work at least when not in fullscreen

            [task launch];
            if (![self serverRunning]) [NSApp terminate:self]; // if there is a server then don't exit!
        NS_HANDLER
            //NSLog(@"%@", localException);
            NSBeginCriticalAlertSheet(
                [NSLocalizedString(@"ClientAlertTitle", @"") expand] , nil, nil, nil,
                window, nil, nil, nil, nil,
                [NSLocalizedString(@"ClientAlertMesg", @"") expand]);
            okay = NO;
        NS_ENDHANDLER
    }

    return okay;
}

/*
 * nil will just launch the fps game
 * "-rpg" will launch the rpg demo
 * "-x.." will launch and run commands
 * otherwise specifying a map to play
 */
- (BOOL)playFile:(id)filename 
{   
    NSUserDefaults *defs = [NSUserDefaults standardUserDefaults];
    
    NSArray *res = [[resolutions titleOfSelectedItem] componentsSeparatedByString:@" x "];  
    NSMutableArray *args = [NSMutableArray array];
    
    [args addObject:[NSString stringWithFormat:@"-dw%@", [res objectAtIndex:0]]];
    [args addObject:[NSString stringWithFormat:@"-dh%@", [res objectAtIndex:1]]];
    [args addObject:@"-dz32"]; // otherwise seems to have a fondness to use -z16 which looks crap
    
    if ([defs integerForKey:dkFULLSCREEN] == 0) [args addObject:@"-df"];
    [args addObject:[NSString stringWithFormat:@"-da%d", [defs integerForKey:dkFSAA]]];
    [args addObject:[NSString stringWithFormat:@"-du%d", [defs integerForKey:dkSHADER]]];
    
    [args addObject:[NSString stringWithFormat:@"-h%@", userPath]];

    NSMutableArray *cmds = [NSMutableArray array];
    if (forcename) [cmds addObject:[NSString stringWithFormat:@"name \"%@\"", NSUserName()]];
    
    if (filename) 
    {
        if ([filename isEqualToString:@"-rpg"]) 
        {
            [cmds removeAllObjects]; // rpg current doesn't require name/team
            [args addObject:@"-grpg"]; //demo the rpg game
        } 
        else if ([filename hasPrefix:@"-x"])
            [cmds addObject:[filename substringFromIndex:2]];
        else 
            [args addObject:[NSString stringWithFormat:@"-l%@", filename]];
    }
    
    if ([cmds count] > 0) 
    {
        NSString *script = [cmds objectAtIndex:0];
        int i;
        for(i = 1; i < [cmds count]; i++) script = [NSString stringWithFormat:@"%@;%@", script, [cmds objectAtIndex:i]];
        [args addObject:[NSString stringWithFormat:@"-x%@", script]];
    }
    
    NSEnumerator *e = [[[defs nonNullStringForKey:dkADVANCEDOPTS] componentsSeparatedByString:@" "] objectEnumerator];
    NSString *opt;
    while(opt = [e nextObject]) if ([opt length] != 0) [args addObject:opt]; //skip empty ones

    return [self launchGame:args];
}

- (void)scanMaps:(id)obj //@note threaded!
{
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    int i;
    for(i = 0; i < 2; i++) 
    {
        NSString *dir = (i==0) ? dataPath : userPath;
        NSDirectoryEnumerator *enumerator = [[NSFileManager defaultManager] enumeratorAtPath:dir];
        NSString *file;
        while(file = [enumerator nextObject]) 
        {
            NSString *role = [fileRoles objectForKey:[file pathExtension]];
            if (role) 
            {   
                Map *map = [[Map alloc] initWithPath:[dir stringByAppendingPathComponent:file] user:(i==1) demo:[role isEqualToString:@"Viewer"]];
                [maps performSelectorOnMainThread:@selector(addObject:) withObject:map waitUntilDone:YES];
                [map release];
            }
        }
    }
    [prog performSelectorOnMainThread:@selector(stopAnimation:) withObject:nil waitUntilDone:NO];
    [pool release];
}

- (void)initMaps 
{
    [prog startAnimation:nil];
    [maps removeObjects:[maps arrangedObjects]];
    [NSThread detachNewThreadSelector: @selector(scanMaps:) toTarget:self withObject:nil];
}

- (void)awakeFromNib 
{
    [self initPaths];
    
    // generate some pretty icons if they are missing
    NSSize size = NSMakeSize(32, 32);
    NSImage *image = [NSImage imageNamed:tkMAIN];
    if (!image) 
    {
        image = [[NSImage imageNamed:@"NSApplicationIcon"] copy];
        [image setSize:size];
        [image setName:tkMAIN]; // one less image to include
    }
    image = [NSImage imageNamed:tkMAPS];
    [image setSize:size];
    
    [self initToolBar];
    [window setBackgroundColor:[NSColor colorWithDeviceRed:0.90 green:0.90 blue:0.90 alpha:1.0]]; // Apple 'mercury' crayon color

    // from the plist determine that dmo->Viewer, and ogz->Editor  
    fileRoles = [[NSMutableDictionary dictionary] retain];
    NSEnumerator *types = [[[NSBundle mainBundle] objectForInfoDictionaryKey:@"CFBundleDocumentTypes"] objectEnumerator];
    NSDictionary *type;
    while((type = [types nextObject])) 
    {
        NSString *role = [type objectForKey:@"CFBundleTypeRole"];
        NSEnumerator *exts = [[type objectForKey:@"CFBundleTypeExtensions"] objectEnumerator];
        NSString *ext;
        while((ext = [exts nextObject])) [fileRoles setObject:role forKey:ext];
    }
    
    NSUserDefaults *defs = [NSUserDefaults standardUserDefaults];
    NSString *appVersion = [[NSBundle mainBundle] objectForInfoDictionaryKey:@"CFBundleVersion"];
    NSString *version = [defs stringForKey:dkVERSION];
    if (!version || ![version isEqualToString:appVersion]) 
    {
        NSLog(@"Upgraded Version...");
        //need to flush lurking config files - they're automatically generated, so no big deal...
        NSString *dir = userPath;
        NSFileManager *fm = [NSFileManager defaultManager];
        [fm removeFileAtPath:[dir stringByAppendingPathComponent:@"init.cfg"] handler:nil];
        [fm removeFileAtPath:[dir stringByAppendingPathComponent:@"config.cfg"] handler:nil];
    }
    [defs setObject:appVersion forKey:dkVERSION];
    
    NSDictionary *dict = [self readConfigFiles];
    [keys addObjects:[self getKeys:dict]];
    
    //encourage people not to remain unnamed
    NSString *name = [dict objectForKey:@"name"];
    forcename = (!name || [name isEqualToString:@""] || [name isEqualToString:@"unnamed"]);
        
    [self initMaps];
    [self initResolutions];
    server = -1;
    [self killServer];
    
    [NSApp setDelegate:self]; //so can catch the double-click, dropped files, termination
    [[NSAppleEventManager sharedAppleEventManager] setEventHandler:self andSelector:@selector(getUrl:withReplyEvent:) forEventClass:kInternetEventClass andEventID:kAEGetURL];    
}


#pragma mark -
#pragma mark application delegate

- (void)applicationDidFinishLaunching:(NSNotification *)note 
{
    if (!dataPath || !appPath) 
    {
        NSBeginCriticalAlertSheet(
            [NSLocalizedString(@"InitAlertTitle", @"") expand], nil, nil, nil,
            window, self, nil, nil, nil,
            [NSLocalizedString(@"InitAlertMesg", @"") expand]);
        NSLog(@"dataPath = '%@'", dataPath);
        NSLog(@"appPath  = '%@'", appPath);
    }
}

-(BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)theApplication 
{
    return YES;
}

- (void)applicationWillTerminate: (NSNotification *)note 
{
    [self killServer];
}

// plist registers 'ogz' and 'dmo' as doc types
- (BOOL)application:(NSApplication *)theApplication openFile:(NSString *)filename 
{
    NSString *role = [fileRoles objectForKey:[filename pathExtension]];
    if (!role) return NO;
    BOOL demo = [role isEqualToString:@"Viewer"];
    filename = [filename stringByDeletingPathExtension]; //chop off extension
    int i;
    for (i = 0; i < 2; i++) 
    {
        NSString *pkg = (i == 0) ? dataPath : userPath;
        if (!demo) pkg = [pkg stringByAppendingPathComponent:@"data/maps"];
        if ([filename hasPrefix:pkg]) return [self playFile:(demo ? [NSString stringWithFormat:@"-xdemo \"%@\"", filename] : filename)];
    }
    NSBeginCriticalAlertSheet(
        [NSLocalizedString(@"FileAlertMesg", @"") expand], NSLocalizedString(@"Ok", @""), NSLocalizedString(@"Cancel", @""), nil,
        window, self, @selector(openPackageFolder:returnCode:contextInfo:), nil, nil,
        [NSLocalizedString(@"FileAlertMesg", @"") expand]);
    return NO;
}

- (void)openPackageFolder:(NSWindow *)sheet returnCode:(int)returnCode contextInfo:(void *)contextInfo 
{
    if (returnCode != 0) [self openUserdir:nil]; 
}

// plist registers 'bloodfrontier' as a url scheme
- (void)getUrl:(NSAppleEventDescriptor *)event withReplyEvent:(NSAppleEventDescriptor *)replyEvent
{
    NSURL *url = [NSURL URLWithString:[[event paramDescriptorForKeyword:keyDirectObject] stringValue]];
    if (!url) return;
    [self playFile:[NSString stringWithFormat:@"-xconnect %@", [url host]]]; 
}

#pragma mark interface actions

- (IBAction)multiplayerAction:(id)sender 
{ 
    [window makeFirstResponder:window]; //ensure fields are exited and committed
    if ([self serverRunning]) 
    {
        [self killServer]; 
    }
    else 
    {
        NSUserDefaults *defs = [NSUserDefaults standardUserDefaults];
    
        NSMutableArray *args = [NSMutableArray arrayWithObject:@"-ss3"];

        NSEnumerator *e = [[[defs nonNullStringForKey:dkSERVEROPTS] componentsSeparatedByString:@" "] objectEnumerator];
        NSString *opt;
        while(opt = [e nextObject]) if ([opt length] != 0) [args addObject:opt]; //skip empty ones
        
        NSString *desc = [defs nonNullStringForKey:dkDESCRIPTION];
        if (![desc isEqualToString:@""]) [args addObject:[NSString stringWithFormat:@"-sd%@", desc]];
        
        NSString *pass = [defs nonNullStringForKey:dkPASSWORD];
        if (![pass isEqualToString:@""]) [args addObject:[NSString stringWithFormat:@"-sP%@", pass]];
        
        int clients = [defs integerForKey:dkMAXCLIENTS];
        if (clients > 0) [args addObject:[NSString stringWithFormat:@"-sc%d", clients]];
        
        [args addObject:[NSString stringWithFormat:@"-h%@", userPath]];
        
        [self launchGame:args];
    } 
}

- (IBAction)playAction:(id)sender 
{ 
    [window makeFirstResponder:window]; //ensure fields are exited and committed
    [self playFile:nil]; 
}

- (IBAction)playRpg:(id)sender 
{ 
    [self playFile:@"-rpg"]; 
}

- (IBAction)playMap:(id)sender
{
    NSArray *sel = [maps selectedObjects];
    if (sel && [sel count] > 0) [self playFile:[[sel objectAtIndex:0] path]];
}

- (IBAction)openUserdir:(id)sender 
{
    [[NSWorkspace sharedWorkspace] openFile:userPath];
}

@end
