#!/usr/bin/perl
#
# Baatvakta project
#

use strict;

# for debugging
use Data::Dumper;

# for round robin database
use RRDs;

# for str2time, time2str
use HTTP::Date;

# handling of stdin and/or file
use IO::Handle;

# handle command line options
use Getopt::Std;

# maria/mysql interface
use DBI;

use IO::Socket::INET;


my $DEBUG;


MAIN:{
    my $connect;
    undef $connect;

##################
# option handling
##################
#   print usage if there is an unknown
#   parameter or help option is passed
    my %options;
    my $ok = getopts('dhn:p:t:', \%options);
    if ( !$ok or $options{h}) {
        Usage();
    }

    if (exists($options{n})){
	makerrds($options{n}, str2time("2016-10-01 00:00:00"));
    }
    if (exists($options{p})){
	runServer($options{p});
    }
    if (exists($options{t})){
	my $t = str2time("2016-10-01 00:00:00") + $options{t};
	updateTemp("veslefrikk", $t, 10.0, 11.0, 12.0, 13.0);
 	updateBat("veslefrikk", $t, 12.2, 24.5);
 	updatePumps("veslefrikk", $t, 100.0, 200.0);
    }
    if (exists($options{d})){
        $DEBUG = $options{d};
    }
#    print "debug: $DEBUG\n";
#    print Dumper({%options});
}



sub runServer{
    my ($port) = @_;
    my @pm;

    # auto-flush on socket
    $| = 1;
 
    # creating a listening socket
    my $socket = new IO::Socket::INET (
	LocalHost => '0.0.0.0',
	LocalPort => $port,
	Proto => 'tcp',
	Listen => 5,
	Reuse => 1
	);
    die "cannot create socket $!\n" unless $socket;
    print "server waiting for client connection on port $port\n";
    
    while(1){
	# waiting for a new client connection
	my $client_socket = $socket->accept();
	
	# get information about a newly connected client
	my $client_address = $client_socket->peerhost();
	my $client_port = $client_socket->peerport();
	print "connection from $client_address:$client_port\n";
	
	# read up to 1024 characters from the connected client
	my $data = "";
	$client_socket->recv($data, 1024);
	print "received data: $data\n";
	@pm = split(/;/,$data);
	updateTemp("veslefrikk", $pm[0], $pm[1], $pm[2], $pm[3], $pm[4]);
 	updateBat("veslefrikk", $pm[0], $pm[5], $pm[6]);
 	updatePumps("veslefrikk", $pm[0], $pm[7], $pm[8]);
	
	# write response data to the connected client
	$data = "ok";
	$client_socket->send($data);
	
	# notify client that response has been sent
	shutdown($client_socket, 1);
    }
    
    $socket->close();
}



# just print usage info
sub Usage {
    print STDERR "Usage: $0 [-d] [-h] [-n name] [-p port]\n";
    exit( 1 );
}


sub makerrds{
# $fname: name
# $start: epoch seconds of first record set
    my ($fname, $start) = @_;
#    my ($fname) = @_;

    RRDs::create ("Temperatures.$fname.rrd",
		  "--start", $start,
		  "--step", "600",
		  "DS:Temperature_Cabin:GAUGE:1200:-30:80",
		  "DS:Temperature_Engine:GAUGE:1200:-30:100",
		  "DS:Temperature_Aft:GAUGE:1200:-30:80",
		  "DS:Temperature_Outside:GAUGE:1200:-30:80",
		  "RRA:AVERAGE:0.5:1:8640",
		  "RRA:AVERAGE:0.5:6:8640",
		  "RRA:MIN:0.5:6:8640",
		  "RRA:MAX:0.5:6:8640",
		  "RRA:AVERAGE:0.5:36:1440",
		  "RRA:MIN:0.5:36:1440",
		  "RRA:MAX:0.5:36:1440");

    RRDs::create ("Batteries.$fname.rrd",
		  "--start", $start,
		  "--step", "600",
		  "DS:Voltage_12V:GAUGE:1200:0:20",
		  "DS:Voltage_24V:GAUGE:1200:0:30",
		  "RRA:AVERAGE:0.5:1:8640",
		  "RRA:AVERAGE:0.5:6:8640",
		  "RRA:MIN:0.5:6:8640",
		  "RRA:MAX:0.5:6:8640",
		  "RRA:AVERAGE:0.5:36:1440",
		  "RRA:MIN:0.5:36:1440",
		  "RRA:MAX:0.5:36:1440");

    RRDs::create ("Pumps.$fname.rrd",
		  "--start", $start,
		  "--step", "600",
		  "DS:EngineRoom:ABSOLUTE:1200:0:600",
		  "DS:Aft:ABSOLUTE:1200:0:600",
#		  "RRA:AVERAGE:0.5:1:8640",
		  "RRA:AVERAGE:0.5:6:8640",
		  "RRA:MIN:0.5:6:8640",
		  "RRA:MAX:0.5:6:8640",
		  "RRA:AVERAGE:0.5:36:1440",
		  "RRA:MIN:0.5:36:1440",
		  "RRA:MAX:0.5:36:1440");

}

sub updateTemp{
    my ($fname, $time, $tCabin, $tEngine, $tAft, $tOutside) = @_;

    RRDs::update ("Temperatures.$fname.rrd", "$time:$tCabin:$tEngine:$tAft:$tOutside");
    my $ERR=RRDs::error;
    if ($ERR) {
	print "ERROR while updating Temperatures.$fname.rrd: $ERR\n"; 
    }
}

sub updateBat{
    my ($fname, $time, $V12, $V24) = @_;

    RRDs::update ("Batteries.$fname.rrd", "$time:$V12:$V24");
    my $ERR=RRDs::error;
    if ($ERR) {
	print "ERROR while updating Batteries.$fname.rrd: $ERR\n"; 
    }
}

sub updatePumps{
    my ($fname, $time, $pEngineDuration, $pAftDuration) = @_;

    RRDs::update ("Pumps.$fname.rrd", "$time:$pEngineDuration:$pAftDuration");
    my $ERR=RRDs::error;
    if ($ERR) {
	print "ERROR while updating Pumps.$fname.rrd: $ERR\n"; 
    }
}



