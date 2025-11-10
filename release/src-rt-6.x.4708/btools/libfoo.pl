#!/usr/bin/perl
#
# libfoo.pl
# Copyright (C) 2006-2008 Jonathan Zarate
# Fixes/updates (C) 2018 - 2024 pedro
#
# - strip un-needed objects
# - create xref of symbols used
#


#use strict;
#use warnings;


my $root = $ENV{"TARGETDIR"};
my $router = $ENV{"SRCBASE"} . "/router";
my $openssldir = $ENV{"OPENSSLDIR"};
my $is_arm = $ENV{"CONFIG_BCMWL6A"};
my $uclibc_lib = $ENV{"TOOLCHAIN"} . "/lib";
my $uclibc_bin = $ENV{"TOOLCHAIN"} . "/bin";
my $stripshared;

sub error
{
	print STDERR "\n*** ERROR: " . (shift) . "\n\n";
	exit 1;
}

sub basename
{
	my $fn = shift;
	if ($fn =~ /([^\/]+)$/) {
		return $1;
	}
	return $fn;
}

sub load
{
	my $fname = shift;

	if ((-l $fname) ||
		($fname =~ /\/lib\/modules\/\d+\.\d+\.\d+/) ||
		($fname =~ /\.(asp|html|gif|png|ico|svg|js|jsx|jsz|css|txt|pat|sh|conf|cnf|csv|gcom|crt|rules|dat|fw|sql)$/)) {
		return;
	}

	if (-d $fname) {
		my $d;
		if (opendir($d, $fname)) {
			foreach (readdir($d)) {
				if ($_ !~ /^\./) {
					load($fname . "/" . $_);
				}
			}
			closedir($d);
		}
		return;
	}

	my $f;
	my $base;
	my $ok;
	my $s;

	$base = basename($fname);
	print LOG "\n\nreadelf $base:\n";

	if ($is_arm eq "y") {
		open($f, $uclibc_bin . "/arm-brcm-linux-uclibcgnueabi-readelf -WhsdD ${fname} 2>&1 |") || error("readelf - $!\n");
	}
	else {
		open($f, $uclibc_bin . "/mipsel-linux-readelf -WhsdD ${fname} 2>&1 |") || error("readelf - $!\n");
	}

	while (<$f>) {
		print LOG;

		if (/\s+Type:\s+(\w+)/) {
			$elf_type{$base} = $1;
			$ok = 1;
			last;
		}
	}

	if (!$ok) {
		close($f);
		return;
	}

	print "$elf_type{$base} $base", " " x 30, "\n";

	push(@elfs, $base);

	while (<$f>) {
		print LOG;

		if (/\(NEEDED\)\s+Shared library: \[(.+)\]/) {
			push(@{$elf_lib{$base}}, $1);
		}
		elsif (/Symbol table for image:/) {
			last;
		}
	}

	while (<$f>) {
		print LOG;

		if (/\s+(WEAK|GLOBAL)\s+(?:DEFAULT|VISIBLE|PROTECTED)\s+(\w+)\s+(\w+)/) {
			$s = $3;
			if ($2 eq 'UND') {
				if ($1 eq 'GLOBAL') {
					$elf_ext{$base}{$s} = 1;
				}
				else {
					print LOG "*** not GLOBAL\n";
				}
			}
			elsif ($2 eq 'ABS') {
			}
			elsif ($2 =~ /^\d+$/) {
				$elf_exp{$base}{$s} = 1;
			}
			else {
				print LOG "*** unknown type\n";
			}
		}
		elsif (!/Num Buc:/) {
			print LOG "*** strange line\n";
		}
	}

	close($f);
}

sub fixDynDep
{
	my ($user, $dep) = @_;
	
	if (!defined $elf_dyn{$user}{$dep}) {
		push(@{$elf_lib{$user}}, $dep);
		$elf_dyn{$user}{$dep} = 1;

		print LOG "FixDynDep: $user = $dep\n";
	}
}

sub fixDyn
{
	print LOG "\nStarting fixDyn ...\n";

	foreach (@elfs) {
		if (/^libipt_.+\.so$/) {
			fixDynDep("iptables", $_);
			if ($is_arm eq "y") {
				fixDynDep($_, "libxtables.so");
			}
		}
		elsif (/^libip6t_.+\.so$/) {
			fixDynDep("ip6tables", $_);
			if ($is_arm eq "y") {
				fixDynDep($_, "libxtables.so");
			}
		}
		elsif (/^libxt_.+\.so$/) {
			fixDynDep("ip6tables", $_);
			if ($is_arm eq "y") {
				fixDynDep($_, "libxtables.so");
			}
		}
		elsif (/^CP\d+\.so$/) {
			fixDynDep("smbd", $_);
		}
	}

	fixDynDep("pppd", "pppol2tp.so");
	fixDynDep("pppd", "pptp.so");
	fixDynDep("pppd", "rp-pppoe.so");

	fixDynDep("libcrypto.so.1.0.0", "libssl.so.1.0.0");
	fixDynDep("libcurl.so.4.8.0", "libwolfssl.so.42.2.0");

	fixDynDep("transmission-daemon", "libevent-2.1.so.7");
	fixDynDep("transmission-daemon", "libcurl.so.4.8.0");
	fixDynDep("transmission-daemon", "libwolfssl.so.42.2.0");
	fixDynDep("transmission-remote", "libevent-2.1.so.7");
	fixDynDep("transmission-remote", "libcurl.so.4.8.0");
	fixDynDep("transmission-remote", "libwolfssl.so.42.2.0");
	fixDynDep("miniupnpd", "libnfnetlink.so.0.2.0");
	fixDynDep("tincd", "liblzo2.so.2.0.0");
	fixDynDep("tincd", "liblz4.so.1.10.0");
	fixDynDep("openvpn", "liblzo2.so.2.0.0");
	fixDynDep("openvpn", "libcap-ng.so.0.0.0");
	fixDynDep("openvpn", "liblz4.so.1.10.0");
	fixDynDep("openvpn", "libwolfssl.so.42.2.0");
	fixDynDep("usb_modeswitch", "libusb-1.0.so");
	fixDynDep("libpcreposix.so.0.0.7", "libpcre.so.1.2.13");
	fixDynDep("libjpeg.so.8.3.2", "libc.so.0");
	fixDynDep("libavcodec.so.54", "libpthread.so.0");

	fixDynDep("minidlna", "libz.so.1");
	fixDynDep("minidlna", "libstdc.so.6");
	fixDynDep("minidlna", "libiconv.so.2.7.0");
	fixDynDep("minidlna", "libjpeg.so.8.3.2");
	fixDynDep("minidlna", "libogg.so.0");
	fixDynDep("minidlna", "libvorbis.so.0");
	fixDynDep("minidlna", "libid3tag.so.0.16.3");
	fixDynDep("minidlna", "libexif.so.12");
	fixDynDep("minidlna", "libFLAC.so.14.0.0");
	fixDynDep("minidlna", "libsqlite3.so.3.51.0");
	fixDynDep("mysql", "libz.so.1");
	fixDynDep("mysqld", "libz.so.1");
	fixDynDep("mysqldump", "libz.so.1");
	fixDynDep("myisamchk", "libz.so.1");
	fixDynDep("mysqladmin", "libz.so.1");
	fixDynDep("my_print_defaults", "libz.so.1");
	fixDynDep("mysql", "libncurses.so.6");
	fixDynDep("mysqld", "libncurses.so.6");
	fixDynDep("mysqldump", "libncurses.so.6");
	fixDynDep("myisamchk", "libncurses.so.6");
	fixDynDep("mysqladmin", "libncurses.so.6");
	fixDynDep("my_print_defaults", "libncurses.so.6");
	fixDynDep("mysql", "libz.so.1");
	fixDynDep("mysqld", "libz.so.1");
	fixDynDep("mysqldump", "libz.so.1");
	fixDynDep("myisamchk", "libz.so.1");
	fixDynDep("mysqladmin", "libz.so.1");
	fixDynDep("my_print_defaults", "libz.so.1");
	fixDynDep("mysql", "libcrypto.so.1.0.0");
	fixDynDep("mysqld", "libcrypto.so.1.0.0");
	fixDynDep("mysqldump", "libcrypto.so.1.0.0");
	fixDynDep("myisamchk", "libcrypto.so.1.0.0");
	fixDynDep("mysqladmin", "libcrypto.so.1.0.0");
	fixDynDep("my_print_defaults", "libcrypto.so.1.0.0");
	fixDynDep("mysql", "libssl.so.1.0.0");
	fixDynDep("mysqld", "libssl.so.1.0.0");
	fixDynDep("mysqldump", "libssl.so.1.0.0");
	fixDynDep("myisamchk", "libssl.so.1.0.0");
	fixDynDep("mysqladmin", "libssl.so.1.0.0");
	fixDynDep("my_print_defaults", "libssl.so.1.0.0");

# ipset modules
	fixDynDep("libipset_iphash.so", "ipset");
	fixDynDep("libipset_iptree.so", "ipset");
	fixDynDep("libipset_ipmap.so", "ipset");
	fixDynDep("libipset_ipporthash.so", "ipset");
	fixDynDep("libipset_ipportiphash.so", "ipset");
	fixDynDep("libipset_ipportnethash.so", "ipset");
	fixDynDep("libipset_iptreemap.so", "ipset");
	fixDynDep("libipset_macipmap.so", "ipset");
	fixDynDep("libipset_nethash.so", "ipset");
	fixDynDep("libipset_portmap.so", "ipset");
	fixDynDep("libipset_setlist.so", "ipset");
	fixDynDep("ipset", "libipset.so.11.1.0");
	fixDynDep("ipset", "libmnl.so.0.2.0");

	fixDynDep("tomatodata.cgi", "libc.so.0");
	fixDynDep("tomatoups.cgi", "libc.so.0");
	fixDynDep("apcupsd", "libc.so.0");
	fixDynDep("apcupsd", "libgcc_s.so.1");
	fixDynDep("apcaccess", "libc.so.0");
	fixDynDep("smtp", "libc.so.0");

	fixDynDep("nginx", "libpcre.so.1.2.13");
	fixDynDep("nginx", "libpcreposix.so.0.0.7");
	fixDynDep("nginx", "libpcre2-8.so.0.15.0");
	fixDynDep("nginx", "libpcre2-posix.so.3.0.7");
	fixDynDep("nginx", "libwolfssl.so.42.2.0");
	if ($is_arm ne "y") { # MIPS
		fixDynDep("php-cgi", "libxml2.so.2.13.7");
	}
	else {
		fixDynDep("php-cgi", "libxml2.so.16.1.1");
	}
	fixDynDep("php-cgi", "libpng16.so.16.50.0");
	fixDynDep("php-cgi", "libiconv.so.2.7.0");
	fixDynDep("php-cgi", "libsqlite3.so.3.51.0");
	fixDynDep("php-cgi", "libcurl.so.4.8.0");
	fixDynDep("php-cgi", "libjpeg.so.8.3.2");
	fixDynDep("php-cgi", "libpcre.so.1.2.13");
	fixDynDep("php-cgi", "libpcreposix.so.0.0.7");
	fixDynDep("php-cgi", "libpcre2-8.so.0.15.0");
	fixDynDep("php-cgi", "libpcre2-posix.so.3.0.7");
	fixDynDep("php-cgi", "libzip.so.5.5");
	if ($is_arm ne "y") { # MIPS
		fixDynDep("php-cli", "libxml2.so.2.13.7");
	}
	else {
		fixDynDep("php-cli", "libxml2.so.16.1.1");
	}
	fixDynDep("php-cli", "libpng16.so.16.50.0");
	fixDynDep("php-cli", "libiconv.so.2.7.0");
	fixDynDep("php-cli", "libsqlite3.so.3.51.0");
	fixDynDep("php-cli", "libcurl.so.4.8.0");
	fixDynDep("php-cli", "libjpeg.so.8.3.2");
	fixDynDep("php-cli", "libpcre.so.1.2.13");
	fixDynDep("php-cli", "libpcreposix.so.0.0.7");
	fixDynDep("php-cli", "libpcre2-8.so.0.15.0");
	fixDynDep("php-cli", "libpcre2-posix.so.3.0.7");
	fixDynDep("php-cli", "libzip.so.5.5");
	if ($is_arm ne "y") { # MIPS
		fixDynDep("php-fpm", "libxml2.so.2.13.7");
	}
	else {
		fixDynDep("php-fpm", "libxml2.so.16.1.1");
	}
	fixDynDep("php-fpm", "libpng16.so.16.50.0");
	fixDynDep("php-fpm", "libiconv.so.2.7.0");
	fixDynDep("php-fpm", "libsqlite3.so.3.51.0");
	fixDynDep("php-fpm", "libcurl.so.4.8.0");
	fixDynDep("php-fpm", "libjpeg.so.8.3.2");
	fixDynDep("php-fpm", "libpcre2-8.so.0.15.0");
	fixDynDep("php-fpm", "libpcre2-posix.so.3.0.7");
	fixDynDep("php-fpm", "libzip.so.5.5");

	fixDynDep("curl", "libcurl.so.4.8.0");
	fixDynDep("mdu", "libcurl.so.4.8.0");

# broadcom WL driver
	fixDynDep("libbcmcrypto.so", "libc.so.0");
	fixDynDep("nas", "libbcmcrypto.so");
	fixDynDep("wl", "libbcmcrypto.so");
	fixDynDep("nas", "libc.so.0");
	fixDynDep("wl", "libc.so.0");

# NocatSplash
	fixDynDep("splashd", "libglib-1.2.so.0.0.10");
	fixDynDep("splashd", "libglib-2.0.so.0.7400.7");

# iperf
	fixDynDep("iperf", "libiperf.so.0.0.0");

# ebtables
	fixDynDep("ebtables-legacy", "libebtc.so.0.0.0");

# samba3
	fixDynDep("samba_multicall", "libiconv.so.2.7.0");

# tor
	fixDynDep("tor", "libevent-2.1.so.7");

# e2fsprogs
	fixDynDep("e2fsck", "libext2fs.so.2.4");
	fixDynDep("e2fsck", "libuuid.so.1.2");
	fixDynDep("e2fsck", "libblkid.so.1.0");
	fixDynDep("e2fsck", "libe2p.so.2.3");
	fixDynDep("e2fsck", "libcom_err.so.2.1");
	fixDynDep("mke2fs", "libext2fs.so.2.4");
	fixDynDep("mke2fs", "libuuid.so.1.2");
	fixDynDep("mke2fs", "libblkid.so.1.0");
	fixDynDep("mke2fs", "libe2p.so.2.3");
	fixDynDep("mke2fs", "libcom_err.so.2.1");
	fixDynDep("tune2fs", "libext2fs.so.2.4");
	fixDynDep("tune2fs", "libuuid.so.1.2");
	fixDynDep("tune2fs", "libblkid.so.1.0");
	fixDynDep("tune2fs", "libe2p.so.2.3");
	fixDynDep("tune2fs", "libcom_err.so.2.1");
	fixDynDep("badblocks", "libext2fs.so.2.4");
	fixDynDep("badblocks", "libuuid.so.1.2");
	fixDynDep("badblocks", "libblkid.so.1.0");
	fixDynDep("badblocks", "libe2p.so.2.3");
	fixDynDep("badblocks", "libcom_err.so.2.1");

# avahi
	fixDynDep("avahi-daemon", "libavahi-core.so.7.1.0");
	fixDynDep("avahi-daemon", "libavahi-common.so.3.5.4");
	fixDynDep("avahi-daemon", "libexpat.so.1.11.1");
	fixDynDep("avahi-daemon", "libdaemon.so.0.5.0");

	fixDynDep("xtables-legacy-multi", "libxtables.so");
	fixDynDep("xtables-legacy-multi", "libip4tc.so");
	fixDynDep("xtables-legacy-multi", "libip6tc.so");

	fixDynDep("libnetfilter_queue.so.1.5.0", "libmnl.so.0.2.0");
	fixDynDep("libnetfilter_queue.so.1.5.0", "libnfnetlink.so.0.2.0");
	fixDynDep("libnetfilter_conntrack.so.3.8.0", "libnfnetlink.so.0.2.0");
	fixDynDep("libnetfilter_log.so.1.2.0", "libnfnetlink.so.0.2.0");
	fixDynDep("libnetfilter_log.so.1.2.0", "libmnl.so.0.2.0");
	fixDynDep("conntrack", "libmnl.so.0.2.0");
	fixDynDep("conntrack", "libnetfilter_conntrack.so.3.8.0");
	fixDynDep("libxt_connlabel.so", "libnetfilter_conntrack.so.3.8.0");

	fixDynDep("irqbalance", "libglib-1.2.so.0.0.10");
	fixDynDep("irqbalance", "libglib-2.0.so.0.7400.7");

	fixDynDep("libipset.so.11.1.0", "libmnl.so.0.2.0");
	fixDynDep("libglib-2.0.so.0.7400.7", "libiconv.so.2.7.0");
	fixDynDep("libglib-2.0.so.0.7400.7", "libpcre2-8.so.0.15.0");
	fixDynDep("libglib-2.0.so.0.7400.7", "libpcre2-posix.so.3.0.7");

	fixDynDep("benchmark", "libwolfssl.so.42.2.0");
	fixDynDep("libmssl.so", "libwolfssl.so.42.2.0");
}

sub usersOf
{
	my $name = shift;
	my $sym = shift;
	my @x;
	my $e;
	my $l;

	@x = ();
	foreach $e (@elfs) {
		foreach $l (@{$elf_lib{$e}}) {
			if ($l eq $name) {
				if ((!defined $sym) || (defined $elf_ext{$e}{$sym})) {
					push(@x, $e);
				}
				last;
			}
		}
	}
	return @x;
}

sub resolve
{
	my $name = shift;
	my $sym = shift;
	my $l;
	
	foreach $l (@{$elf_lib{$name}}) {
#		print "\n$l $sym ", $elf_exp{$l}{$sym}, "\n";
		return $l if (defined $elf_exp{$l}{$sym});
	}
	return "*** unresolved ***";
}

sub fillGaps
{
	my $name;
	my $sym;
	my @users;
	my $u;
	my $t;
	my $found;

#	print "\nfillGaps: Resolving implicit links...\n";
	print LOG "\nfillGaps: Resolving implicit links...\n";

	foreach $name (@elfs) {
		foreach $sym (keys %{$elf_ext{$name}}) {
			$found = 0;

			if ($sym eq '__uClibc_start_main') {
				$sym = '__uClibc_main';
			}

			#  __gnu_local_gp is defined specially by the linker on MIPS
			if ($sym eq '__gnu_local_gp') {
				$found = 1;
			}
			elsif (resolve($name, $sym) eq "*** unresolved ***") {
				@users = usersOf($name);
				foreach $u (@users) {
					# if exported by $u
					if (defined $elf_exp{$u}{$sym}) {
						fixDynDep($name, $u);
						$found = 1;
					}
					# if exported by shared libs of $u
					if (($t = resolve($u, $sym)) ne "*** unresolved ***") {
						fixDynDep($name, $t);
						$found = 1;
					}
				}

				if ($found == 0) {
					print "Unable to resolve $sym used by $name\n", @users;
					exit 1;
				}
			}
		}
	}
}

sub tab
{
	my $current = shift;
	my $target = shift;
	my $s = "";
	my $n;

	while (1) {
		$n = $current + (4 - ($current % 4));
		last if ($n > $target);
		$s = $s . "\t";
		$current = $n;
	}
	while ($current < $target) {
		$s = $s . " ";
		$current++;
	}
	return $s;
}

sub genXref
{
	my $f;
	my $fname;
	my $s;
	my @u;

	print "Generating Xref Report...\n\n";

	open($f, ">libfoo_xref.txt");
	foreach $fname (sort keys %elf_type) {
		print $f "$fname:\n";

		if (scalar(@{$elf_lib{$fname}}) > 0) {
			print $f "Dependency:\n";
			foreach $s (sort @{$elf_lib{$fname}}) {
				print $f "\t$s", defined $elf_dyn{$fname}{$s} ? " (dyn)\n" : "\n";
			}
		}

		if (scalar(keys %{$elf_exp{$fname}}) > 0) {
			print $f "Export:\n";
			foreach $s (sort keys %{$elf_exp{$fname}}) {
				@u = usersOf($fname, $s);
				if (scalar(@u) > 0) {
					print $f "\t$s", tab(length($s) + 4, 40), " > ", join(",", @u), "\n";
				}
				else {
					print $f "\t$s\n";
				}
			}
		}

		if (scalar(keys %{$elf_ext{$fname}}) > 0) {
			print $f "External:\n";
			foreach $s (sort keys %{$elf_ext{$fname}}) {
				print $f "\t$s", tab(length($s) + 4, 40), " < ", resolve($fname, $s), "\n";
			}
		}
		
		print $f "\n";
	}
	close($f);
}


sub genSO
{
	my ($so, $arc, $strip, $opt) = @_;
	my $name = basename($so);
	my $sym;
	my $fn;
	my $inuse;
	my @used;
	my @unused;
	my $cmd;
	my $before, $after;

	if (!-f $so) {
		print "$name: not found, skipping...\n";
		return 0;
	}

	if (!-f $arc) {
		print "$arc: not found, skipping...\n";
		return 0;
	}
	
	foreach $sym (sort keys %{$elf_exp{$name}}) {
		if ((scalar(usersOf($name, $sym)) > 0) || (${strip} eq "no")) {
			push(@used, $sym);
		}
		else {
			push(@unused, $sym);
		}
	}

#	print "\n$name: Attempting to link ", scalar(@used), " and remove ", scalar(@unused), " objects...\n";

	print LOG "\n\n${base}\n";

	if ($is_arm eq "y") {
		$cmd = $uclibc_bin . "/arm-brcm-linux-uclibcgnueabi-gcc -shared -nostdlib -Wl,-s,-z,combreloc -Wl,--warn-common -Wl,--gc-sections ${opt} -Wl,-soname=${name} -o ${so}";
	}
	else {
		$cmd = $uclibc_bin . "/mipsel-uclibc-gcc -shared -nostdlib -Wl,-s,-z,combreloc -Wl,--warn-common -Wl,--fatal-warnings -Wl,--gc-sections ${opt} -Wl,-soname=${name} -o ${so}";
	}

	foreach (@{$elf_lib{$name}}) {
		if ((!$elf_dyn{$name}{$_}) && (/^lib(.+)\.so/)) {
			$cmd .= " -l$1";
		}
		else {
#			print "$_: not marked for linkage...\n";
		}
	}
#	print "$cmd -u... ${arc}\n";
	if (scalar(@used) == 0) {
		print "$name: WARNING: Library is not used by anything, deleting...\n";
		unlink $so;
		return 0;
	}
	$cmd .= " -u " . join(" -u ", @used) . " ". $arc;

	print LOG "Command: $cmd\n";
	print LOG "Used: ", join(",", @used), "\n";
	print LOG "Unused: ", join(",", @unused), "\n";
	
	$before = -s $so;

	system($cmd);
	if ($? != 0) {
		error("gcc returned $?");
	}

	$after = -s $so;

	print "$name: Attempted to remove ", scalar(@unused), "/", scalar(@unused) + scalar(@used), " symbols. ";
	printf "%.2fK - %.2fK = %.2fK\n", $before / 1024, $after / 1024, ($before - $after) / 1024;

#	print "\n$name: Attempting to link ", scalar(@used), " and remove ", scalar(@unused), " objects...\n";
#	printf "Before: %.2fK / After: %.2fK / Removed: %.2fK\n\n", $before / 1024, $after / 1024, ($before - $after) / 1024;

	return ($before > $after)
}


##
##
##

print "\nlibfoo.pl - fooify shared libraries\n";
print "Copyright (C) 2006-2007 Jonathan Zarate\n";
print "Fixes/updates (C) 2018 - 2024 pedro\n\n";

if ((!-d $root) || (!-d $router) || (!-d $uclibc_bin) || (!-d $uclibc_lib)) {
	print "Missing or invalid environment variables\n";
	exit(1);
}

#open(LOG, ">libfoo.debug");
open(LOG, ">/dev/null");

print "\n--- Loading files... ---\n\n";
load($root);
print "\n--- Finished loading files. ---\n\n";

fixDyn();
fillGaps();
genXref();

$stripshared = "yes";
if ($ARGV[0] eq "--noopt") {
	$stripshared = "no";
}

if ($is_arm ne "y") { # MIPS only
	genSO("${root}/lib/libc.so.0", "${uclibc_lib}/libc.a", "", "-Wl,-init=__uClibc_init ${uclibc_lib}/optinfo/interp.os");
	genSO("${root}/lib/libresolv.so.0", "${uclibc_lib}/libresolv.a", "${stripshared}");
	genSO("${root}/lib/libcrypt.so.0", "${uclibc_lib}/libcrypt.a", "${stripshared}");
	genSO("${root}/lib/libm.so.0", "${uclibc_lib}/libm.a");
	genSO("${root}/lib/libpthread.so.0", "${uclibc_lib}/libpthread.a", "${stripshared}", "-u pthread_mutexattr_init -Wl,-init=__pthread_initialize_minimal_internal");
	genSO("${root}/lib/libutil.so.0", "${uclibc_lib}/libutil.a", "${stripshared}");
	#genSO("${root}/lib/libdl.so.0", "${uclibc_lib}/libdl.a", "${stripshared}");
	#genSO("${root}/lib/libnsl.so.0", "${uclibc_lib}/libnsl.a", "${stripshared}");
	genSO("${root}/usr/lib/libbcmcrypto.so", "${router}/libbcmcrypto/libbcmcrypto.a");
}

if ($openssldir eq "openssl") {
	genSO("${root}/usr/lib/libcrypto.so.1.0.0", "${router}/${openssldir}/libcrypto.a", "${stripshared}");
	genSO("${root}/usr/lib/libssl.so.1.0.0", "${router}/${openssldir}/libssl.a", "${stripshared}", "-L${router}/${openssldir}");
}
elsif ($openssldir eq "openssl-1.1") {
	if ($is_arm ne "y") { # ARM - error (TBCH)
		genSO("${root}/usr/lib/libcrypto.so.1.1", "${router}/${openssldir}/libcrypto.a", "${stripshared}");
	}
	# libssl - bigger
}
elsif ($openssldir eq "openssl-3.0") {
	if ($is_arm ne "y") { # ARM - error (TBCH)
		genSO("${root}/usr/lib/libcrypto.so.3", "${router}/${openssldir}/libcrypto.a", "${stripshared}");
	}
	# libssl - bigger
}

#genSO("${root}/usr/lib/libwolfssl.so.42.2.0", "${router}/wolfssl/src/.libs/libwolfssl.a", "${stripshared}");

if ($is_arm ne "y") { # MIPS only (ARM - bigger)
	genSO("${root}/usr/lib/libid3tag.so.0.16.3", "${router}/libid3tag/static/libid3tag.a", "${stripshared}", "-L${router}/zlib");
	genSO("${root}/usr/lib/libexif.so.12", "${router}/libexif/libexif/.libs/libexif.a", "${stripshared}");
	genSO("${root}/usr/lib/libavcodec.so.54", "${router}/ffmpeg//libavcodec/libavcodec.a", "${stripshared}", "-L${router}/ffmpeg/libavutil -L${router}/zlib");
	genSO("${root}/usr/lib/libavformat.so.54", "${router}/ffmpeg/libavformat/libavformat.a", "${stripshared}", "-L${router}/ffmpeg/libavutil -L${router}/ffmpeg/libavcodec -L${router}/zlib");
}

genSO("${root}/usr/lib/libexpat.so.1.11.1", "${router}/expat/static/libexpat.a", "${stripshared}");
genSO("${root}/usr/lib/libavutil.so.51", "${router}/ffmpeg/libavutil/libavutil.a", "${stripshared}", "-L${router}/zlib");
genSO("${root}/usr/lib/libzebra.so", "${router}/zebra/lib/libzebra.a", "${stripshared}");
genSO("${root}/usr/lib/libnfnetlink.so.0.2.0", "${router}/libnfnetlink/src/.libs/libnfnetlink.a", "${stripshared}");
genSO("${root}/usr/lib/libdaemon.so.0.5.0", "${router}/libdaemon/libdaemon/.libs/libdaemon.a", "${stripshared}");
genSO("${root}/usr/lib/libmnl.so.0.2.0", "${router}/libmnl/src/.libs/libmnl.a", "${stripshared}");
genSO("${root}/usr/lib/libjpeg.so.8.3.2", "${router}/libjpeg-turbo/build/libjpeg.a", "${stripshared}");
genSO("${root}/usr/lib/libvorbis.so.0", "${router}/libvorbis/lib/.libs/libvorbis.a", "${stripshared}", "-L${router}/libogg/src/.libs");
genSO("${root}/usr/lib/libFLAC.so.14.0.0", "${router}/flac/src/libFLAC/.libs/libFLAC.a", "${stripshared}", "-L${router}/libogg/src/.libs");
genSO("${root}/usr/lib/libogg.so.0", "${router}/libogg/src/.libs/libogg.a", "${stripshared}");
genSO("${root}/usr/lib/libz.so.1", "${router}/zlib/libz.a", "${stripshared}");
genSO("${root}/usr/lib/liblzo2.so.2.0.0", "${router}/lzo/src/.libs/liblzo2.a", "${stripshared}");
genSO("${root}/usr/lib/liblz4.so.1.10.0", "${router}/lz4/lib/liblz4.a", "${stripshared}");
genSO("${root}/usr/lib/libcurl.so.4.8.0", "${router}/libcurl/lib/.libs/libcurl.a", "${stripshared}", "-L${router}/zlib -L${router}/${openssldir} -L${router}/wolfssl/staged/usr/lib");
genSO("${root}/usr/lib/libevent-2.1.so.7", "${router}/libevent/.libs/libevent.a", "${stripshared}");
genSO("${root}/usr/lib/libiconv.so.2.7.0", "${router}/libiconv/lib/.libs/libiconv.a", "${stripshared}");
genSO("${root}/usr/lib/libpng16.so.16.50.0", "${router}/libpng/staged/usr/lib/libpng16.a", "${stripshared}", "-L${router}/zlib");
if ($is_arm ne "y") { # MIPS
	genSO("${root}/usr/lib/libxml2.so.2.13.7", "${router}/libxml2/staged/usr/lib/libxml2.a", "${stripshared}", "-L${router}/zlib");
}
else {
	genSO("${root}/usr/lib/libxml2.so.16.1.1", "${router}/libxml2/staged/usr/lib/libxml2.a", "${stripshared}", "-L${router}/zlib");
}
genSO("${root}/usr/lib/libpcre.so.1.2.13", "${router}/pcre/.libs/libpcre.a", "${stripshared}");
genSO("${root}/usr/lib/libpcre2-8.so.0.15.0", "${router}/pcre2/staged/usr/lib/libpcre2-8.a", "${stripshared}");
#genSO("${root}/usr/lib/libpcre2-posix.so.3.0.7", "${router}/pcre2/staged/usr/lib/libpcre2-posix.a", "${stripshared}"); # (deleted)
genSO("${root}/usr/lib/libiperf.so.0.0.0", "${router}/iperf/src/.libs/libiperf.a", "${stripshared}");
genSO("${root}/usr/lib/libncurses.so.6", "${router}/libncurses/lib/libncurses.a", "${stripshared}");

genSO("${root}/usr/lib/libavahi-common.so.3.5.4", "${router}/avahi/avahi-common/.libs/libavahi-common.a", "${stripshared}");
genSO("${root}/usr/lib/libavahi-core.so.7.1.0", "${router}/avahi/avahi-core/.libs/libavahi-core.a", "${stripshared}", "-L${router}/avahi/avahi-common/.libs");

genSO("${root}/usr/lib/libext2fs.so.2.4", "${router}/e2fsprogs/lib/libext2fs.a", "${stripshared}", "-L${router}/e2fsprogs/lib");
genSO("${root}/usr/lib/libuuid.so.1.2", "${router}/e2fsprogs/lib/libuuid.a", "${stripshared}", "-L${router}/e2fsprogs/lib");
genSO("${root}/usr/lib/libblkid.so.1.0", "${router}/e2fsprogs/lib/libblkid.a", "${stripshared}", "-L${router}/e2fsprogs/lib");
genSO("${root}/usr/lib/libe2p.so.2.3", "${router}/e2fsprogs/lib/libe2p.a", "${stripshared}", "-L${router}/e2fsprogs/lib");
genSO("${root}/usr/lib/libcom_err.so.2.1", "${router}/e2fsprogs/lib/libcom_err.a", "${stripshared}", "-L${router}/e2fsprogs/lib");

#genSO("${root}/usr/lib/libipset.so.11.1.0", "${router}/ipset/lib/.libs/libipset.a", "${stripshared}"); # (deleted)
#genSO("${root}/usr/lib/libffi.so.8.2.0", "${router}/libffi/.libs/libffi.a", "${stripshared}"); # (deleted)

# do not add (!): libcap-ng, libzip, libsqlite3, libebtc, libiptc, libxtables, libip4tc, libip6tc, libglib-1, libglib-2, ...


print "\n--- end ---\n\n";

close(LOG);
exit(0);
