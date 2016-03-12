<?php
/*
 * This class contains the menu generating supporting scripts
 *
 * Author: Bismay Kumar Mohapatra bismay4u@gmail.com
 * Version: 1.0
 */
if(!defined('ROOT')) exit('No direct script access allowed');

class MenuGenerator {
	
	private $dataSources=[];
	private $defaultData=[
				"id"=>"",
				"menuid"=>"files",
				"title"=>"",
				"mode"=>"*",
				"category"=>"",
				"menugroup"=>"/",
				"class"=>"",
				"target"=>"",
				"link"=>"#",
				"iconpath"=>"",
				"tips"=>"",
				"site"=>"*",
				"device"=>"*",
				"privilege"=>"*",
				"weight"=>"100",
				"onmenu"=>"true",
				"blocked"=>"false",
				"rules"=>"",
			];

	public function __construct($masterMenugroup="Masters",$dataSources=['app']) {
		$this->defaultData['menugroup']=$masterMenugroup;
		if(is_array($dataSources)) $this->dataSources=$dataSources;
		else $this->dataSources[]=[$dataSources];

		$this->dataSources=array_unique($this->dataSources);

		$ds=[];
		foreach ($this->dataSources as $key) {
			$tbls=_db($key)->get_tableList();
			foreach ($tbls as $a=>$b) {
				$ds[$b]=$key;
			}
		}
		$this->dataSources=$ds;
		//printArray($this->dataSources);exit();
	}

	public function processMenuDB($navKey="default",$dbTable=null,$dbKey="app") {
		$dbTable=_dbTable($dbTable, $dbKey);

		$site=SITENAME;
		$pid=getMyRoleHash();
		$cols=_db($dbKey)->get_columnList($dbTable);

		$sql=_db($dbKey)->_selectQ($dbTable,"*",[
					"blocked"=>"false",
					"onmenu"=>"true",
					"menuid"=>$navKey
				]);
		$sql->_where("(site='$site' OR site='*')");
		
		if(isset($_COOKIE['USER_DEVICE']) && in_array("device", $cols)) {
			$sql->_where("(device='*' OR FIND_IN_SET('{$_COOKIE['USER_DEVICE']}',device))");
		}

		if(_session("SESS_PRIVILEGE_ID")!=1) {

		}
		$sql->_where("(find_in_set('$pid',privilege) OR privilege='*')");

		if(in_array("weight", $cols)) {
			$sql->_orderby("weight ASC");
		}
		
		//echo $sql->_SQL();
		$data=$sql->_get();

		return $data;
	}

	public function processMenuDir($dirPath,$menugroup=null) {
		if(!is_dir($dirPath)) return [];
		if($menugroup==null) $menugroup=$this->defaultData['menugroup'];

		$fs=scandir($dirPath);
		$fs=array_splice($fs, 2);

		$fData=[];
		foreach ($fs as $f) {
			if(is_dir($dirPath.$f)) {
				$data=$this->processMenuDir($dirPath.$f."/",$f);
				foreach ($data as $key => $subMenu) {
					$fData[]=$subMenu;
				}
			} else {
				$data=json_decode(file_get_contents($dirPath.$f),true);
				$data['id']=md5($dirPath.$f);
				$data['menugroup']=$menugroup;
				$fData[]=array_merge($this->defaultData,$data);
			}
		}
		return $fData;
	}

	public function processMenuRules($data) {
		//"module","dbtable",(TODO "page","dbcolumn")
		foreach ($data as $key => $menuData) {
			if(isset($menuData['rules'])) {
				$flds=explode(",",$menuData['rules']);
				foreach($flds as $toCheck) {
					$ar=explode("#",$toCheck);
					if(count($ar)>=2) {
						switch ($ar[0]) {
							case 'module':
								if(!checkModule($ar[1])) {
									unset($data[$key]);
									break;
								}
								break;
							case 'dbtable':
								if(!array_key_exists($ar[1], $this->dataSources)) {
									unset($data[$key]);
									break;
								}
								break;

						}
					}
				}
			}
		}

		return $data;
	}

	public function processMenuArray($data) {
		//printArray($data);exit();

		$data=$this->processMenuRules($data);

		$menuList=[];
		$fData=[];
		foreach ($data as $id => $item) {
			$menuList[$item['id']]=$item;
			if($item['menugroup']=="/") {
				if(!isset($fData[$item['title']])) {
					$fData[$item['title']]=[];
				}
			}
		}
		
		foreach ($menuList as $id => $item) {
			$group=$item['menugroup'];
			if(is_numeric($group)) {
				if(isset($menuList[$group])) $group=$menuList[$group]['title'];
				else $group="/";
			}

			if(strlen($item['link'])>0 && ($item['link']!="#" || $item['link']!="##")) {
				$item['link']=_link($item['link']);
			}

			if($group!="/") {
				if(!isset($fData[$group])) $fData[$group]=[];
				$fData[$group][$item['id']]=$item;
			}
		}
		return $fData;
	}

	public function __debugInfo() {
        return [];
    }
}
?>