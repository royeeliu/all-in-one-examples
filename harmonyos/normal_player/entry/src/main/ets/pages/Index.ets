import { common } from '@kit.AbilityKit';
import  { picker } from '@kit.CoreFileKit';
import { BusinessError } from '@kit.BasicServicesKit';
import media from '@ohos.multimedia.media';
import fs from '@ohos.file.fs';

@Entry
@Component
struct Index {
  tag: string = 'HomePage';
  private xComponentController: XComponentController = new XComponentController();
  private surfaceId: string = '';
  private avPlayer: media.AVPlayer | null = null;

  @State filePath: string = '';

  async onSelectFile(context: common.UIAbilityContext) {
    try {
      let documentSelectOptions = new picker.DocumentSelectOptions();
      let documentPicker = new picker.DocumentViewPicker(context);
      documentPicker.select(documentSelectOptions).then((documentSelectResult: Array<string>) => {
        console.info(`${this.tag}: DocumentViewPicker.select successfully, documentSelectResult uri: ${JSON.stringify(documentSelectResult)}`);
        this.filePath = documentSelectResult.length > 0 ? documentSelectResult[0] : '';
      }).catch((err: BusinessError) => {
        console.error(`${this.tag}: DocumentViewPicker.select failed with err, code is: ${err.code}, message is: ${err.message}`);
      });
    } catch (error) {
      let err: BusinessError = error as BusinessError;
      console.error(`${this.tag}: DocumentViewPicker failed with err, code is: ${err.code}, message is: ${err.message}`);
    }
  }

  async setupAVPlayer() {
    this.avPlayer = await media.createAVPlayer();
    this.avPlayer.on('error', (error) => {
      console.error(`${this.tag}: avplayer failed, code: ${error.code}, msg: ${error.message}`);
    });
    this.avPlayer.on('stateChange', async (state: string, reason: media.StateChangeReason) => {
      console.log(`${this.tag}: stateChange state: ${state}`);
      if (this.avPlayer == null) {
        console.info(`${this.tag}: avPlayer has not init on state change`);
        return;
      }

      switch (state) {
        case 'initialized': {
          // surfaceId 属性只能在 initialized 状态设置
          this.avPlayer.surfaceId = this.surfaceId;
          // prepare() 方法只能在 initialized 状态调用
          this.avPlayer.prepare();
        }
        break;
        case 'prepared': {
          this.avPlayer.play();
        }
        break;
      }
    });

    // 不能将文件路径直接赋值给 url 属性，可以使用 `fd://${fd}` 的形式，第二个 fd 是对应文件的文件描述符 FD
    // 或者构造 AVFileDescriptor 赋值给 fdSrc 属性
    let file = await fs.open(this.filePath, fs.OpenMode.READ_ONLY);
    // 直接将 File 赋值给 fdSrc 就可以，因为构造 AVFileDescriptor 只有 fd 属性是必需的，
    // offset 和 length 有默认值，File 对象有 fd 属性，所以编译器自动提取了 fd 属性并构造
    // AVFileDescriptor 对象
    this.avPlayer.fdSrc = file;
    await fs.close(file);
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
              this.setupAVPlayer();
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