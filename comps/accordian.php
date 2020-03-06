<?php
loadModuleLib("navigator","api");

$menuTree1=generateNavigationFromDB("default","admin_links","app");

$menuTree2=generateNavigationFromDir(APPROOT."misc/menus/","app");

$menuTree=array_merge_recursive($menuTree1,$menuTree2);
//printArray($menuTree);exit();
?>
<style>
.sidebarMenu {
  width: 100%;
  overflow-x: auto;
  position: absolute;
  bottom: -20px;
  top: 32px;
  left: 0px;
  right: 0px;
}
</style>
<div class="panel-group sidebarMenu" id="accordion" role="tablist" aria-multiselectable="true">
  <?php
    foreach ($menuTree as $category=>$menuSet) {
      if(count($menuSet)<=0) continue;
      $hash=md5($category);
      echo "<div class='panel panel-default'>";
      echo "  <div class='panel-heading' role='tab' id='$hash'>";
      echo "    <h4 class='panel-title'>";
      echo "      <a role='button' data-toggle='collapse' data-parent='#accordion' href='#collapse{$hash}' aria-expanded='true' aria-controls='collapseOne'>";
      echo "        $category";
      echo "      </a>";
      echo "      <i class='fa fa-angle-right pull-right'></i>";
      echo "    </h4>";
      echo "  </div>";
      echo "  <div id='collapse{$hash}' class='panel-collapse collapse' role='tabpanel' aria-labelledby='$hash'>";
      echo "    <div class='panel-body'>";
      
      foreach ($menuSet as $key => $menu) {
        $more=[];
        if($menu['target']!=null && strlen($menu['target'])>0) {
          $more[]="target='{$menu['target']}'";
        }
        if($menu['class']!=null && strlen($menu['class'])>0) {
          $more[]="class='menuItem {$menu['class']}'";
        }
        if($menu['category']!=null && strlen($menu['category'])>0) {
          $more[]="data-category='{$menu['category']}'";
        }
        if($menu['tips']!=null && strlen($menu['tips'])>0) {
          $more[]="title='{$menu['tips']}'";
        }
        if($menu['iconpath']!=null && strlen($menu['iconpath'])>0) {
          echo "<a href='{$menu['link']}' ".implode(" ", $more)."><i class='menuIcon {$menu['iconpath']}'></i>&nbsp; {$menu['title']}</a>";
        } else {
          echo "<a href='{$menu['link']}' ".implode(" ", $more).">{$menu['title']}</a>";
        }
      }

      echo "    </div>";
      echo "  </div>";
      echo "</div>";
    }
  ?>
</div>