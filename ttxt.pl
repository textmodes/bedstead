#! /usr/bin/perl

$ylimit = 6; $xlimit = 8;

@pixel = (
	  [ 0, 0, 0, 1, 0, 0 ],
	  [ 0, 0, 1, 0, 1, 0 ],
	  [ 0, 1, 0, 0, 0, 1 ],
	  [ 0, 1, 0, 0, 0, 1 ],
	  [ 0, 1, 0, 0, 0, 1 ],
	  [ 0, 0, 1, 0, 1, 0 ],
	  [ 0, 0, 0, 1, 0, 0 ],
	  [ 0, 0, 0, 0, 0, 0 ],
	  );

@augment = ( [], [], [], [], [], [], [] );

for ($x = 0; $x < $xlimit - 1; $x++) {
    for ($y = 0; $y < $ylimit - 1; $y++) {
	if ($pixel[$x][$y] == $pixel[$x+1][$y+1] &&
	    $pixel[$x][$y] != $pixel[$x+1][$y] &&
	    $pixel[$x][$y+1] == $pixel[$x+1][$y]) {
	    $augment[$x][$y] = 1;
	}
    }
}

for ($x = 0; $x < $xlimit; $x++) {
    for ($y = 0; $y < $ylimit; $y++) {
	print "gsave $x $y translate\n";
	print "[ ";
	print $augment[$x-1][$y-1] ? "true " : "false ";
	print $augment[$x][$y-1] ? "true " : "false ";
	print $augment[$x][$y] ? "true " : "false ";
	print $augment[$x-1][$y] ? "true " : "false ";
	print "] ";
	print $pixel[$x][$y] ? "blackpixel\n" : "whitepixel\n";
	print "grestore\n";
    }
}
