#!/usr/bin/perl

my $denom = $ARGV[0];

for ( $i = $j = 0, $bs = 80; <stdin>; $i++ ){
	my $place = $i * $bs / $denom;

	if ( $place > $j ){
		print( "\r[", "-"x$place, " "x($bs-$place), "] " );
		$j = $place;
	}
}

print( "\n" );
