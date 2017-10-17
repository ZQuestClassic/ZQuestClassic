#!/usr/bin/perl
# convert template from menus.txt to HTML and wikitext


$infile = "menus.txt";
$outhtml = "menus.html";
$outwiki = "menus.wiki";

$status = "normal";

open(INPUT,$infile) || die "Input file won't open.";
open(OUTHTML,">" . $outhtml) || die "Html file won't open.";
open(OUTWIKI,">" . $outwiki) || die "Wiki file won't open.";

while(<INPUT>) {
	if(/^#/) { # comment line
		next;
	}
	elsif(/^END/) {
		$status = "end";
		close(INPUT);
	} elsif (/^\s*$/ && $status eq "normal") { # blank
		$status = "blank";
	}

	if($status eq "normal") {
		$text = $_;
		$text =~ s/^>*//;
		chomp $text; 
		$text =~ s/\x0d$//;
		if(/^>>>>/) {
			print OUTHTML "<h5>" . $text . "</h5>\n";
			print OUTWIKI "===== " . $text . " =====\n";
		} elsif (/^>>>/) {
			print OUTHTML "<h4>" . $text . "</h4>\n";
			print OUTWIKI "==== " . $text . " ====\n";
		} elsif (/^>>/) {
			print OUTHTML "<h3>" . $text . "</h3>\n";
			print OUTWIKI "=== " . $text . " ===\n";
		} elsif (/^>/) {
			print OUTHTML "<h2>" . $text . "</h2>\n";
			print OUTWIKI "== " . $text . " ==\n";
		} elsif (/^@\(/) { # start quote
			$status = "quote";
		} else { # description
			print OUTHTML "<p>" . $text . "</p>\n";
			print OUTWIKI;
		}
	} elsif ($status eq "quote") {
		if (/^@\)/) {
			$status = "normal";
		} else {
			print OUTHTML;
			# delete it in the wiki as it won't work.
		}
	} elsif ($status eq "blank") {
		$status = "normal";
	}
}
