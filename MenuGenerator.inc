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
		$pid=$_SESSION['SESS_PRIVILEGE_NAME'];
		$phash=getMyRoleHash();
		$cols=_db($dbKey)->get_columnList($dbTable);

		$sql=_db($dbKey)->_selectQ($dbTable,"*",[
					"blocked"=>"false",
					"onmenu"=>"true",
// 					"menuid"=>$navKey
				]);
    	$sql->_whereIN("menuid",$navKey);
		$sql->_whereIN("site",[$site,'*']);//"(site='$site' OR site='*')"

		if(isset($_COOKIE['USER_DEVICE']) && in_array("device", $cols)) {
			$sql->_whereIN("device",['*',$_COOKIE['USER_DEVICE']]);
			//$sql->_whereOR("device",['*',["OP"=>":FIND:","VALUE"=>$_COOKIE['USER_DEVICE']]]);
			//"(device='*' OR FIND_IN_SET('{$_COOKIE['USER_DEVICE']}',device))"
		}

		if(_session("SESS_PRIVILEGE_ID")!=1) {
			$sql->_whereRAW("(find_in_set('$pid',privilege) OR find_in_set('$phash',privilege) OR privilege='*')");
		}

		if(in_array("weight", $cols)) {
			$sql->_orderby("weight ASC");
		}
		//exit($sql->_SQL());
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
			if(strtolower(substr($f,strlen($f)-5))!=".json") continue;
			if(is_dir($dirPath.$f)) {
				$data=$this->processMenuDir($dirPath.$f."/",$f);
				foreach ($data as $key => $subMenu) {
					$fData[]=$subMenu;
				}
			} else {
				$data=json_decode(file_get_contents($dirPath.$f),true);
				$data['id']=md5($dirPath.$f);
				if(!isset($data['menugroup'])) $data['menugroup']=$menugroup;
				$fData[]=array_merge($this->defaultData,$data);
			}
		}
		
		$pid=$_SESSION['SESS_PRIVILEGE_NAME'];
		$phash=getMyRoleHash();
		foreach ($fData as $key => $menu) {
			if($menu['blocked']===true || $menu['blocked']=="true") unset($fData[$key]);
			//Check if Device is Allowed
			if(isset($menu['device']) && $menu['device']!="*" && $menu['device']!="") {
				$temp = explode(",", $menu['device']);
				if(!in_array($_COOKIE['USER_DEVICE'], $temp)) {
					unset($fData[$key]);
				}
			}
			//Check if privilege is Allowed
			if(isset($menu['privilege']) && $menu['privilege']!="*" && $menu['privilege']!="") {
				$temp = explode(",", $menu['privilege']);
				if(!in_array($pid, $temp) && !in_array($phash, $temp)) {
					unset($fData[$key]);
				}
			}
		}
		return $fData;
	}

	public function processMenuRules($data) {
		if(!is_array($data) || empty($data)) return [];
		//"module","dbtable",(TODO "page","dbcolumn")
		foreach ($data as $key => $menuData) {
			if(!isset($menuData['rules']) && isset($menuData['to_check'])) {
				$menuData['rules']=$menuData['to_check'];
			}
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
								if(!checkUserScope($ar[1])) {
									unset($data[$key]);
									break;
								}
								break;
							case "policy":
								if(!checkUserPolicy($ar[1])) {
									unset($data[$key]);
									break;
								}
								break;
							case 'scope':case 'privileges':case 'users':
								$arr=explode(".",$ar[1]);
								if(count($arr)>=3) {
									if(!checkUserRoles($arr[0],$arr[1],$arr[2])) {
										unset($data[$key]);
										break;
									}
								} elseif(count($arr)>=2) {
									if(!checkUserRoles($arr[0],$arr[1])) {
										unset($data[$key]);
										break;
									}
								} else {
									//if(!checkUserScope($ar[1])) {
									if(!checkUserRoles($arr[0], "main")) {
										unset($data[$key]);
										break;
									}
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
		if(!is_array($data) || empty($data)) return [];
		//printArray($data);exit();

		$data=$this->processMenuRules($data);
		//printArray($data);exit();

		$menuList=[];
		$fData=[];
		foreach ($data as $id => $item) {
			$menuList[$item['id']]=$item;
			if($item['menugroup']=="/" || strlen($item['menugroup'])==0) {
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
