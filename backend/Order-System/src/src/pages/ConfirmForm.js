import React, { useState } from "react";
import ReactDOM from "react-dom";

import { Button, Modal } from "react-bootstrap";

const Backdrop = (props) => {
  return <div className="backdrop" onClick={props.onConfirm} />;
};

const ModalOverlay = (props) => {
  return (
    <>
      <Modal show={props.show} onHide={props.onCloseForm} size="lg">
        <Modal.Header closeButton>
          <Modal.Title>Your Order</Modal.Title>
        </Modal.Header>
        <Modal.Body>
          {props.product[0] > 0 && <p>牛肉麵 : {props.product[0]} 份</p>}
          {props.product[1] > 0 && <p>小籠包 : {props.product[1]} 份</p>}
          {props.product[2] > 0 && <p>滷肉飯 : {props.product[2]} 份</p>}
          {props.product[3] > 0 && <p>蚵仔麵線 : {props.product[3]} 份</p>}
          {props.product[4] > 0 && <p>蚵仔煎 : {props.product[4]} 份</p>}
          {props.product[5] > 0 && <p>臭豆腐 : {props.product[5]} 份</p>}
          {props.product[6] > 0 && <p>雞排 : {props.product[6]} 份</p>}
          {props.product[7] > 0 && <p>珍珠奶茶 : {props.product[7]} 份</p>}
          {props.product[8] > 0 && <p>刨冰 : {props.product[8]} 份</p>}
          {props.product[9] > 0 && <p>鳳梨酥 : {props.product[9]} 份</p>}

          <p>Total : NT$ {props.price}</p>
        </Modal.Body>
        <Modal.Footer>
          <Button variant="secondary" onClick={props.onCloseForm}>
            Cancel
          </Button>
          <Button variant="primary" onClick={props.onSend}>
            Confirm
          </Button>
        </Modal.Footer>
      </Modal>
    </>
  );
};

const ConfirmForm = (props) => {
  return (
    <React.Fragment>
      {ReactDOM.createPortal(<Backdrop onConfirm={props.onConfirm} />, document.getElementById("form-root"))}
      {ReactDOM.createPortal(
        <ModalOverlay
          onSetForm={props.onSetForm}
          onCloseForm={props.onCloseForm}
          onSend={props.onSend}
          show={props.show}
          price={props.price}
          product={props.product}
        />,
        document.getElementById("overlay-root"),
      )}
    </React.Fragment>
  );
};

export default ConfirmForm;
