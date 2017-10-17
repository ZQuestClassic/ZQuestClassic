#!/usr/bin/perl
# Perl script for generating HTML and wikitext from zscript.txt
#

# this should go into command line options later
$input = "zscript.txt";
$htmlout = "zscript.html";
$wikiout = "zscript.wiki";

$do_html = true;
$do_wiki = true;

%text = (); # a hash to hold all commands and text of a section for sorting
$current = "";

# status tells us where in the file we are.
# start: before the first section
# section: reached a new section
# command: we're waiting for an actual command/variable
# command2: we saw a command, this is either more of it or the text starts here
# text: the text going with a command

$status = "start";

# open the files
open(INPUT, $input) || die "Input file doesn't exist!";
if (do_html) {
	open(OUTHTML,">" . $htmlout) || die "HTML output file can't be opened."
}
if (do_wiki) {
	open(OUTWIKI,">" . $wikiout) || die "Wiki output file can't be opened."
}

# start off - headings
write_head();

# read the file, line by line
while (<INPUT>) {
	# clean up any < > that will interfere with html. Fix DOS lines.
	s/</&lt;/g; s/>/&gt;/g; s/\x0d//;
	if ($status eq  "start") {
		if(/^\s*$/) { # empty line
		} elsif (/^===/) { # new section
			chomp $data;
			write_first($data);
			$status="section";
		} else { # stuff before the first section
			chomp $data;
			if ($data eq "") {
				$data = $_;
			} else {
				$data = $data . " " . $_;
			}
		}
	} elsif ($status eq  "section") {
		# sections begin like this:
		# ====================
		# --- Section name ---  << we are here!
		# ====================
		s/^--- //; s/---.*$//; # strip line
		chomp;
		write_section($_);
		$_ = <INPUT>; # chomp third line
		write_cmdliststart();		
		%text = (); # clear for new section
		$status = command;
	} elsif ($status eq  "command") {
		# here comes a command, or an empty line, or a new section
		if (/^\s*$/) { # empty
		} elsif (/^===/) { # new section
			# dump the section
			foreach $current (sort by_funcname keys %text) {
				write_cmd($current);
				write_text($text{$current});
			} 
			write_cmdlistend();
			$status = "section";
		} else { # it really is a command
			$data = $_;  
			$status = "command2";
		}
	} elsif ($status eq "command2") {
		if (/^ \* /) { # text starts here
			chomp $data;
			
			$current = $data;
			# write_cmd($data);

			s/^ \* //; # remove the marker
			$data = $_;
			$status = "text";
		} else { # more command ...
			chomp $data;
			$data = $data . " " . $_;
		}
	} elsif ($status eq  "text") { # text begins with " * "
		# gather up text in data until there's an empty line then print it
		if (/^ \* /) {
			s/^ \* //; # remove marker and join onto data
			chomp $data;
			$data = $data . " " . $_;
		} elsif (/^\s*$/) { # empty line - end of text. Print it.
			chomp $data;

			$text{$current} = $data;
			# write_text($data);

			$status = "command";
		} else {
			oops($_);
		}
	} else {
		oops("$status == " . $status);
	}
}

# end of file - clean up
# first of all, is there text?
if ($status eq text) {
	chomp $data;
	$text{$current} = $data;	
}

# last section
foreach $current (sort by_funcname keys %text) {
	write_cmd($current);
	write_text($text{$current});
}
write_cmdlistend(); 
write_end();

print "Done.\n";

# ----------

sub by_funcname {
	# sort by 2nd word
	$A = $a; $B = $b;
	$A =~ s/^\w*//;
	$B =~ s/^\w*//;
	return $A cmp $B;
}

sub write_head {
if (do_html) {
	print OUTHTML "<h1>ZScript Documentation</h1>\n";
}
if (do_wiki) {
	print OUTWIKI "{{version|2.50}}\n";
}
}

sub write_first {
if (do_html) {
	print OUTHTML "<p><i>" . $data . "</i></p>\n";
}
if (do_wiki) {
	print OUTWIKI $data . "\n";
}
}

sub write_section {
if (do_html) {
	print OUTHTML "<h2>" . $_[0] . "</h2>\n";
}

if (do_wiki) {
	print OUTWIKI "\n== " . $_[0] . " ==\n";
}
}

sub write_cmdliststart {
if (do_html) {
	print OUTHTML "<dl>\n";
}
}

sub write_cmdlistend {
if (do_html) {
	print OUTHTML "</dl>\n";
}

}

sub write_cmd {
if (do_html) {
	print OUTHTML "<dt>" . $_[0] . "</dt>\n";
}
if (do_wiki) {
	print OUTWIKI "\n=== " . $_[0] . " ===\n"
}
}

sub write_text {
if (do_html) {
	print OUTHTML "<dd>" . $_[0] . "</dd>\n";
}
if (do_wiki) {
	print OUTWIKI $_[0] . "\n";
}
}

sub write_end {

}

sub oops {
	warn "Something funny happened! Status is <" . $status . ">,\n";
	warn "this was unexpected :  " . $_[0] . "\n";
	die;
}
