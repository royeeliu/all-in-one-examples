import { common } from '@kit.AbilityKit';
import  { picker } from '@kit.CoreFileKit';
import { BusinessError } from '@kit.BasicServicesKit';
import media from '@ohos.multimedia.media';

@Entry
@Component
struct Index {
  tag: string = 'HomePage';
  private xComponentController: XComponentController = new XComponentController();
  private surfaceId: string = '';

  @State filePath: string = '';

  async onSelectFile(context: common.UIAbilityContext) {
    try {
      let documentSelectOptions = new picker.DocumentSelectOptions();
      let documentPicker = new picker.DocumentViewPicker(context);
      documentPicker.select(documentSelectOptions).then((documentSelectResult: Array<string>) => {
        console.info('DocumentViewPicker.select successfully, documentSelectResult uri: ' + JSON.stringify(documentSelectResult));
        this.filePath = documentSelectResult.length > 0 ? documentSelectResult[0] : '';
      }).catch((err: BusinessError) => {
        console.error(`DocumentViewPicker.select failed with err, code is: ${err.code}, message is: ${err.message}`);
      });
    } catch (error) {
      let err: BusinessError = error as BusinessError;
      console.error(`DocumentViewPicker failed with err, code is: ${err.code}, message is: ${err.message}`);
    }
  }

  build() {
    Row() {
      Column() {
        if (this.filePath) {
          XComponent({
            // 装载视频容器
            id: 'xComponent',
            type: XComponentType.SURFACE,
            controller: this.xComponentController
          })
            .id('VideoView')
            .visibility(Visibility.Visible)
            .onLoad(() => {
              this.surfaceId = this.xComponentController.getXComponentSurfaceId();
              console.info(`${this.tag}: surfaceId = ${this.surfaceId}`);
            })
            .width('100%')
            .height('100%')
            .backgroundColor(Color.Black)
        } else {
          Button('选择视频')
            .type(ButtonType.Capsule)
            .onClick(() => {
              let context = this.getUIContext().getHostContext() as common.UIAbilityContext;
              this.onSelectFile(context);
            })
        }
      }.width('100%')
    }.height('100%')
  }
}