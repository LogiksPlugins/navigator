<?php
if(!defined('ROOT')) exit('No direct script access allowed');

if(!function_exists("generateNavigationFromDB")) {

	include __DIR__."/MenuGenerator.inc";

	//Based on Data Tables, create menu and implement the menu system (TODO cache)
	function generateNavigationFromDB($navKey="default",$dbTable=null,$dbKey="app") {
		$mg=new MenuGenerator("Masters",['core',$dbKey]);
		return $mg->processMenuArray($mg->processMenuDB($navKey,$dbTable,$dbKey));
	}

	//Based on JSON files in the provided folder, create menu and implement the menu system (TODO cache)
	function generateNavigationFromDir($dirPath,$dbKey="app") {
		$mg=new MenuGenerator("Masters",['core',$dbKey]);
		return $mg->processMenuArray($mg->processMenuDir($dirPath));
	}
}
?>